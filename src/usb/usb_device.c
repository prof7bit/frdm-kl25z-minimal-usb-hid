/**
 * @file
 * Implementation of an USB HID device on the KL25
 *
 * This implements a full duplex stream over HID protocol,
 * it is intended to work on Windows without the user needing
 * to interact with any driver installation, it should load
 * and use the existing generic HID driver.
 *
 * The IN and OUT streams are tunneled over two 63 byte sized
 * HID reports, each consisting of 1 byte length and up to 62
 * byte data. The application interface is just two FIFO pipes
 * for RX and TX.
 *
 * Additionally a special message packet of 62 byte can be
 * transmitted outside the stream and with higher priority,
 * this can be used for arbitrary control and status purposes.
 *
 * Created on: 02.11.2016
 *     Author: Bernd Kreuss
 *
 * Largely inspired by the initial work of Kevin Cuzner:
 *     http://kevincuzner.com/2014/12/12/teensy-3-1-bare-metal-writing-a-usb-driver/
 *     https://github.com/kcuzner/teensy-oscilloscope
 */

#include <stddef.h>
#include <MKL25Z4.h>

#include "usb_device.h"
#include "fifo.h"

#define ENDPOINT_BUF_SIZE               64
#define USB_NUM_ENDPOINTS               2

#define TOK_OUT                         0x1
#define TOK_IN                          0x9
#define TOK_SOF                         0x5
#define TOK_SETUP                       0xd


#define RX                              0
#define TX                              1
#define EVEN                            0
#define ODD                             1
#define DATA0                           0
#define DATA1                           1
#define BD_BC_SHIFT                     16
#define BD_OWN_MASK                     (1 << 7)
#define BD_DATA1_MASK                   (1 << 6)
#define BD_KEEP_MASK                    (1 << 5)
#define BD_NINC_MASK                    (1 << 4)
#define BD_DTS_MASK                     (1 << 3)
#define BD_STALL_MASK                   (1 << 2)
#define BDT_INDEX(endpoint, tx, odd)    ((endpoint << 2) | (tx << 1) | odd)
#define BD_GET_TOK(desc)                ((desc >> 2) & 0xF)
#define BD_OWNED_BY_USB(count, data1)   ((count << BD_BC_SHIFT) | BD_OWN_MASK | (data1 ? BD_DATA1_MASK : 0x00) | BD_DTS_MASK)

#define REPORT_ID_RX                    2
#define REPORT_ID_TX                    1
#define MAGIC_MESSAGE_PACKET            0xff

#define WEAK                            __attribute((weak))
#define ALIGN512                        __attribute((aligned(512)))


/**
 * Buffer Descriptor Table entry
 * There are two entries per direction per endpoint:
 *   In  Even/Odd
 *   Out Even/Odd
 * A bidirectional endpoint would then need 4 entries
 *
 * The USB-DMA may access the descriptor, the pointer
 * and even the bytes pointed to at any time, therefore
 * they are all declared as volatile.
 */
typedef struct {
    volatile uint32_t desc;
    volatile void* volatile addr;
} buffer_descriptor_t;

/**
 * The double buffered endpoint buffers holding the data for transmission
 * or receiving the data. This type is used for RX and TX buffers, all of
 * them can be read or written either by the the USB-DMA or the application,
 * therefore all their contents are declared as volatile.
 */
typedef volatile uint8_t usb_endpoint_buffer_t[2][ENDPOINT_BUF_SIZE];

/**
 * Structure of a SETUP packet, used by endpoint 0
 */
typedef struct {
    union {
        struct {
            uint8_t bmRequestType;
            uint8_t bRequest;
        };
        uint16_t wRequestAndType;
    };
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} setup_t;

/**
 * Our hid report packets always include a payload size member
 * because due to a bug in the generic Windows HID driver it
 * will always either send a full sized packet or no packet at
 * all, no matter the actual byte count to transmit.
 */
typedef volatile struct {
    uint8_t payload_size;
    uint8_t payload_data[];
} hid_packet_header_t;

typedef struct {
    uint8_t tx_odd;
    uint8_t tx_data1;
} endpoint_state_t;

typedef enum {
    MSG_QUEUED,
    MSG_TRANSMITTING,
    MSG_FREE
} message_packet_state_t;

static endpoint_state_t endpoint_state[USB_NUM_ENDPOINTS] = {};

static usb_endpoint_buffer_t endpoint_0_rx_buf;
static usb_endpoint_buffer_t endpoint_1_rx_buf;
static usb_endpoint_buffer_t endpoint_1_tx_buf;

static volatile message_packet_state_t message_packet_state = MSG_FREE;
static volatile uint8_t message_packet_buffer[64];

static uint8_t tx_fifo_buf[512] = {};
static uint8_t rx_fifo_buf[512] = {};

fifo_t usb_rx;
fifo_t usb_tx;

/**
 * Buffer descriptor table, aligned to a 512-byte boundary
 */
ALIGN512 static buffer_descriptor_t buf_desc_table[USB_NUM_ENDPOINTS * 4];

/*
 * weak empty default implementations of the hook functions
 */
WEAK void usb_hook_led_rx(bool on) {}
WEAK void usb_hook_led_tx(bool on) {}
WEAK void usb_hook_message_packet(volatile uint8_t* data) {}

/**
 * A "message packet" here is nothing USB specific, instead it
 * belongs to my own little stream over HID protocol, all packets
 * with a payload size containing the magic number are considered
 * not part of the streaming data but are part of a separate kind
 * of "control channel" that can exchange messages of up to 63
 * byte per message. These messages have priority over the queued
 * stream data.
 *
 * This function will enqueue such a message by copying the data to
 * its own buffer and return true if successful. It will be sent
 * through the next available IN transaction. If there is already
 * a message queued then nothing happens and the function just
 * returns false, the application must try again later.
 */
bool usb_send_message_packet(uint8_t* data, uint8_t size) {
    if (message_packet_state == MSG_FREE){
        if (size > sizeof(message_packet_buffer) - sizeof(hid_packet_header_t)) {
            size = sizeof(message_packet_buffer) - sizeof(hid_packet_header_t);
        }
        hid_packet_header_t* p = (hid_packet_header_t*)message_packet_buffer;
        p->payload_size = MAGIC_MESSAGE_PACKET;
        for (int i=0; i<size; i++) {
            p->payload_data[i] = data[i];
        }
        message_packet_state = MSG_QUEUED;
        return true;
    }
    return false;
}

static void init_buffer_descriptor(uint8_t endpoint, usb_endpoint_buffer_t buffer, uint8_t buffer_size) {
    endpoint_state[endpoint].tx_odd = EVEN;
    endpoint_state[endpoint].tx_data1 = DATA0;
    buf_desc_table[BDT_INDEX(endpoint, RX, EVEN)].desc = BD_OWNED_BY_USB(buffer_size, DATA0);
    buf_desc_table[BDT_INDEX(endpoint, RX, EVEN)].addr = buffer[EVEN];
    buf_desc_table[BDT_INDEX(endpoint, RX, ODD)].desc = BD_OWNED_BY_USB(buffer_size, DATA1);
    buf_desc_table[BDT_INDEX(endpoint, RX, ODD)].addr = buffer[ODD];
    buf_desc_table[BDT_INDEX(endpoint, TX, EVEN)].desc = 0;
    buf_desc_table[BDT_INDEX(endpoint, TX, ODD)].desc = 0;
    USB0->ENDPOINT[endpoint].ENDPT = USB_ENDPT_EPRXEN_MASK
                                   | USB_ENDPT_EPTXEN_MASK
                                   | USB_ENDPT_EPHSHK_MASK;
}

void usb_device_init(void) {
    uint32_t i;

    //reset the buffer descriptors
    for (i = 0; i < USB_NUM_ENDPOINTS * 4; i++) {
        buf_desc_table[i].desc = 0;
        buf_desc_table[i].addr = 0;
    }

    //1: Select clock source
    SIM->SOPT2 |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK;

    //2: Gate USB clock
    SIM->SCGC4 |= SIM_SCGC4_USBOTG_MASK;

    //3: Software USB module reset
    USB0->USBTRC0 |= USB_USBTRC0_USBRESET_MASK;
    while (USB0->USBTRC0 & USB_USBTRC0_USBRESET_MASK);

    //4: Set BDT base registers
    USB0->BDTPAGE1 = ((uint32_t) buf_desc_table) >> 8;  //bits 15-9
    USB0->BDTPAGE2 = ((uint32_t) buf_desc_table) >> 16; //bits 23-16
    USB0->BDTPAGE3 = ((uint32_t) buf_desc_table) >> 24; //bits 31-24

    //5: Clear all ISR flags and enable weak pull downs
    USB0->ISTAT = 0xFF;
    USB0->ERRSTAT = 0xFF;
    USB0->OTGISTAT = 0xFF;
    USB0->USBTRC0 |= 0x40; //a hint was given that this is an undocumented interrupt bit

    //6: Enable USB reset interrupt
    USB0->CTL = USB_CTL_USBENSOFEN_MASK;
    USB0->USBCTRL = 0;

    USB0->INTEN |= USB_INTEN_USBRSTEN_MASK;
    //NVIC_SET_PRIORITY(IRQ(INT_USB0), 112);
    NVIC_EnableIRQ(USB0_IRQn);

    //7: Enable pull-up resistor on D+ (Full speed, 12Mbit/s)
    USB0->CONTROL = USB_CONTROL_DPPULLUPNONOTG_MASK;

    // initialize FIFO buffers for the stream-over-hid protocol.
    fifo_init(&usb_rx, rx_fifo_buf, sizeof(rx_fifo_buf));
    fifo_init(&usb_tx, tx_fifo_buf, sizeof(tx_fifo_buf));
}

void endpoint_prepare_next_tx(uint8_t endpoint, volatile void* data, uint8_t length) {
    endpoint_state_t* state = &endpoint_state[endpoint];
    buffer_descriptor_t* bd = &buf_desc_table[BDT_INDEX(endpoint, TX, state->tx_odd)];
    bd->addr = data;
    bd->desc = BD_OWNED_BY_USB(length, state->tx_data1);

    //toggle the odd and data bits
    state->tx_data1 ^= 1;
    state->tx_odd ^= 1;
}

void bd_rx_release(buffer_descriptor_t* buf_desc) {
    /*
     * Unfortunately when preparing a new buffer descriptor for
     * reception we MUST specify whether it is meant to receive
     * DATA1 or DATA0. The most straightforward way to achieve
     * this is to just preserve the DATA1 bit in the descriptor
     * as it is, so the even descriptors will always stay DATA0
     * and the odd ones always DATA1 like they have been since
     * they were first initialized during USB reset.
     */
    uint8_t data1 = buf_desc->desc & BD_DATA1_MASK ? 1 : 0;
    buf_desc->desc = BD_OWNED_BY_USB(ENDPOINT_BUF_SIZE, data1);
}

static bool endpoint_have_free_tx_descriptor(uint8_t endpoint) {
    /*
     * We look at the buffer descriptor that would be used in the
     * next transmission and determine whether its own bit is set.
     * If it is still owned by the USB then we cannot send anything
     * at this time, both TX-buffers are full.
     */
    uint32_t desc = buf_desc_table[BDT_INDEX(endpoint, TX, endpoint_state[endpoint].tx_odd)].desc;
    return (desc & BD_OWN_MASK) == 0;
}

static void endpoint_1_check_tx() {
    uint8_t tx_byte;
    if (endpoint_have_free_tx_descriptor(1)) {

        /*
         * Our special message packets have priority
         * over stream data.
         */
        if (message_packet_state == MSG_QUEUED) {
            endpoint_prepare_next_tx(1, message_packet_buffer, 64);
            message_packet_state = MSG_TRANSMITTING;

        /*
         * Check if data is in the TX queue and if so
         * then fill the next TX buffer for sending.
         */
        } else {
            if (fifo_get_size(&usb_tx)) {
                usb_hook_led_tx(true);
                uint8_t odd = endpoint_state[1].tx_odd;
                hid_packet_header_t* p = (hid_packet_header_t*)endpoint_1_tx_buf[odd];
                uint8_t i = 0;
                while ((i < 64 - sizeof(hid_packet_header_t)) && fifo_pop(&usb_tx, &tx_byte)) {
                    p->payload_data[i++] = tx_byte;
                }
                p->payload_size = i;

                /*
                 * Due to a bug in the generic Windows HID driver we must always
                 * either TX a full sized packet or no packet at all, otherwise
                 * the driver would be confused. This is also the reason we need
                 * to waste one byte for the payload size in our reports.
                 */
                endpoint_prepare_next_tx(1, p, 64);
            }
        }
    }
}

static void endpoint_1_handler(uint8_t tok, buffer_descriptor_t* buf_desc) {
    hid_packet_header_t* p;
    uint8_t size;

    switch (tok) {

    case TOK_IN:
        /*
         * check whether the last TX was one of our special message
         * packets and if so mark the message buffer as free again.
         */
        if (message_packet_state == MSG_TRANSMITTING) {
            p = buf_desc->addr;
            if (p->payload_size == MAGIC_MESSAGE_PACKET) {
                message_packet_state = MSG_FREE;
            }
        }

        /*
         * check whether there is still more data left to transmit
         */
        endpoint_1_check_tx();
        break;

    case TOK_OUT:
        usb_hook_led_rx(true);
        p = buf_desc->addr;
        size = buf_desc->desc >> 16;
        if (size > sizeof(hid_packet_header_t)) {
            if (p->payload_size <= size - sizeof(hid_packet_header_t)) {
                /*
                 * all packets with a payload of 0..63 are
                 * interpreted as stream data, the payload data
                 * is extracted and pushed into the RX FIFO.
                 */
                for (int i = 0; i < p->payload_size; ++i) {
                    fifo_push(&usb_rx, p->payload_data[i]);
                }

            } else if (p->payload_size == MAGIC_MESSAGE_PACKET) {
                /*
                 * since there is a range of otherwise invalid
                 * sizes above 62 we can use the size field to
                 * encode a special marker here for a special
                 * type of packet that does not contain stream
                 * data. The application can implement the hook
                 * below to receive them if it so wishes.
                 */
                usb_hook_message_packet(p->payload_data);
            }
        }
        break;
    }
}

static void endpoint_0_handler(uint8_t tok, buffer_descriptor_t* buf_desc) {
    static setup_t setup;
    static uint8_t* remaining_tx_data_ptr = NULL;
    static uint16_t remaining_tx_data_length = 0;

    uint8_t tx_data_length = 0;
    uint8_t* tx_data_ptr = NULL;
    uint32_t tx_size = 0;
    bool must_stall = false;

    switch (tok) {
    case TOK_SETUP:
        /*
         *  We might still need some information about this SETUP
         *  packet even after the next transaction, therefore we
         *  do not just use a pointer to the volatile RX-buffer,
         *  instead we copy the entire contents of the packet into
         *  a static struct.
         */
        setup = *((setup_t*) (buf_desc->addr));
        usb_hook_led_rx(true);

        /*
         * Any SETUP packet implies that there is no more pending
         * IN data expected by the host and the answer has to be
         * transmitted immediately and always starting with a DATA1
         * packet. Therefore we forcefully clear both TX-descriptors
         * and reinitialize the DATA1 toggle.
         */
        buf_desc_table[BDT_INDEX(0, TX, EVEN)].desc = 0;
        buf_desc_table[BDT_INDEX(0, TX, ODD)].desc = 0;
        endpoint_state[0].tx_data1 = DATA1;

        switch (setup.wRequestAndType) {

        case 0x0500: //set address (wait for IN packet)
            break;

        case 0x0900: //set configuration
            //we only have one configuration at this time
            break;

        case 0x0680: //get descriptor
        case 0x0681:
            must_stall = true;
            const unsigned num_descr = sizeof(descriptor_table) / sizeof(descriptor_table_t);
            for (unsigned i=0; i<num_descr; i++) {
                if (setup.wValue == descriptor_table[i].wValue
                &&  setup.wIndex == descriptor_table[i].wIndex) {
                    tx_data_ptr = (uint8_t*)descriptor_table[i].descriptor;
                    tx_data_length = descriptor_table[i].size;
                    must_stall = false;
                    usb_hook_led_tx(true);
                    break;
                }
            }
            break;

        default:
            must_stall = true;
        }

        if (must_stall) {
            USB0->ENDPOINT[0].ENDPT = USB_ENDPT_EPSTALL_MASK
                                    | USB_ENDPT_EPRXEN_MASK
                                    | USB_ENDPT_EPTXEN_MASK
                                    | USB_ENDPT_EPHSHK_MASK;

        } else {
            /*
             * truncate the data length to whatever the setup packet is
             * expecting. if data_length == 0 then this will result in the
             * sending of a zero length packet which is is the correct
             * answer for all requests that do not request any data.
             */
            if (tx_data_length > setup.wLength) {
                tx_data_length = setup.wLength;
            }

            do {
                /*
                 * at this point (after a SETUP) we know that both TX buffer
                 * descriptors are free (we forcefully cleared them a few
                 * lines above because every SETUP discards any old pending
                 * IN data). Therefore we can immediately prepare up to two
                 * buffer descriptors with your answer.
                 */

                // transmit 1st chunk
                tx_size = tx_data_length;
                if (tx_size > ENDPOINT_BUF_SIZE)
                    tx_size = ENDPOINT_BUF_SIZE;
                endpoint_prepare_next_tx(0, tx_data_ptr, tx_size);
                tx_data_ptr += tx_size; //move the pointer down
                tx_data_length -= tx_size; //move the size down
                if (tx_data_length == 0 && tx_size < ENDPOINT_BUF_SIZE)
                    break; //all done!

                // transmit 2nd chunk
                tx_size = tx_data_length;
                if (tx_size > ENDPOINT_BUF_SIZE)
                    tx_size = ENDPOINT_BUF_SIZE;
                endpoint_prepare_next_tx(0, tx_data_ptr, tx_size);
                tx_data_ptr += tx_size; //move the pointer down
                tx_data_length -= tx_size; //move the size down
                if (tx_data_length == 0 && tx_size < ENDPOINT_BUF_SIZE)
                    break; //all done!

                /*
                 * if any data remains to be transmitted after both
                 * TX-descriptors are armed then we need to store it,
                 * the remaining data will be picked up during
                 * subsequent TOK_IN until all data is sent.
                 */
                remaining_tx_data_ptr = tx_data_ptr;
                remaining_tx_data_length = tx_data_length;

            } while (false);
        }

        // must unfreeze after every setup packet
        USB0->CTL = USB_CTL_USBENSOFEN_MASK;
        break;

    case TOK_IN:
        // continue sending any pending transmit data
        tx_data_ptr = remaining_tx_data_ptr;
        if (tx_data_ptr) {
            tx_size = remaining_tx_data_length;
            if (tx_size > ENDPOINT_BUF_SIZE)
                tx_size = ENDPOINT_BUF_SIZE;
            endpoint_prepare_next_tx(0, tx_data_ptr, tx_size);
            tx_data_ptr += tx_size;
            remaining_tx_data_length -= tx_size;
            remaining_tx_data_ptr =
                (remaining_tx_data_length > 0 || tx_size == ENDPOINT_BUF_SIZE) ? tx_data_ptr : NULL;
        }

        // delayed setting of address can happen here
        if (setup.wRequestAndType == 0x0500) {
            USB0->ADDR = setup.wValue;
        }
        break;

    case TOK_OUT:
        break;

    case TOK_SOF:
        break;
    }
}

void USB0_IRQHandler(void) {
    uint8_t status;
    uint8_t stat, endpoint;
    uint8_t tx, odd;

    status = USB0->ISTAT;

    /*
     * reset
     */
    if (status & USB_ISTAT_USBRST_MASK) {
        //handle USB reset

        //initialize endpoint 0 ping-pong buffers
        USB0->CTL |= USB_CTL_ODDRST_MASK;
        init_buffer_descriptor(0, endpoint_0_rx_buf, ENDPOINT_BUF_SIZE);

        // initialize endpoint 1
        init_buffer_descriptor(1, endpoint_1_rx_buf, ENDPOINT_BUF_SIZE);

        //clear all interrupts...this is a reset
        USB0->ERRSTAT = 0xff;
        USB0->ISTAT = 0xff;

        //after reset, we are address 0, per USB spec
        USB0->ADDR = 0;

        //all necessary interrupts are now active
        USB0->ERREN = 0xFF;
        USB0->INTEN = USB_INTEN_USBRSTEN_MASK | USB_INTEN_ERROREN_MASK
            | USB_INTEN_SOFTOKEN_MASK | USB_INTEN_TOKDNEEN_MASK
            | USB_INTEN_SLEEPEN_MASK | USB_INTEN_STALLEN_MASK;
        return;
    }

    /*
     * error
     */
    if (status & USB_ISTAT_ERROR_MASK) {
        //handle error
        uint8_t est = USB0->ERRSTAT;
        USB0->ERRSTAT = est;
        USB0->ISTAT = USB_ISTAT_ERROR_MASK;
    }

    /*
     * start of frame
     */
    if (status & USB_ISTAT_SOFTOK_MASK) {
        //turn off all LEDs again
        usb_hook_led_rx(false);
        usb_hook_led_tx(false);

        /*
         * periodically poll endpoint 1 so it can check whether the
         * application has placed anything in its transmit queue.
         *
         * This is called from here because the token handler would
         * never be called again once the TX buffers have run dry and
         * the hardware automatically NAKs every subsequent TOK_IN.
         */
        endpoint_1_check_tx();

        USB0->ISTAT = USB_ISTAT_SOFTOK_MASK;
    }

    /*
     * token completed
     *
     * This means a transaction has completed, we receive this interrupt
     * after data was transferred and the handshake has been completed:
     *
     *   host:TOK_IN    -> device:DATA ->   host:ACK/NAK -> Interrupt
     *   host:TOK_OUT   ->   host:DATA -> device:ACK/NAK -> Interrupt
     *   host:TOK_SETUP ->   host:DATA -> device:ACK/NAK -> Interrupt
     *
     * The handler can then process the received data or queue more
     * data to be sent during the next transaction. When there is a
     * TOK_IN transaction and no data has been placed in the TX buffer
     * then the hardware will automatically send a NAK:
     *
     *   host:TOK_IN -> device:NAK -> NO Interrupt!
     *
     * There will be no interrupt in this case! This means as long
     * as there is no data to be sent there will also be no TOK_IN
     * calls to the handler anymore. Something else has to prepare
     * a new TX buffer for that endpoint when data becomes available
     * again, the handler will only be called AFTER the transmission!
     */
    if (status & USB_ISTAT_TOKDNE_MASK) {
        //handle completion of current token being processed
        stat = USB0->STAT;
        endpoint = stat >> 4;
        tx = (stat & USB_STAT_TX_MASK) >> USB_STAT_TX_SHIFT;
        odd = (stat & USB_STAT_ODD_MASK) >> USB_STAT_ODD_SHIFT;

        //determine which buffer descriptor we are looking at here
        buffer_descriptor_t* buf_desc = &buf_desc_table[BDT_INDEX(endpoint, tx, odd)];

        // determine which token has been processed
        uint8_t tok = BD_GET_TOK(buf_desc->desc);

        if (endpoint == 0) {
            endpoint_0_handler(tok, buf_desc);
        } else if(endpoint == 1) {
            endpoint_1_handler(tok, buf_desc);
        }

        if (!tx) {
            // give RX buffer back
            bd_rx_release(buf_desc);
        }

        USB0->ISTAT = USB_ISTAT_TOKDNE_MASK;
    }

    /*
     * sleep
     */
    if (status & USB_ISTAT_SLEEP_MASK) {
        //handle USB sleep
        USB0->ISTAT = USB_ISTAT_SLEEP_MASK;
    }

    /*
     * stall
     */
    if (status & USB_ISTAT_STALL_MASK) {
        //handle usb stall
        USB0->ISTAT = USB_ISTAT_STALL_MASK;
    }
}
