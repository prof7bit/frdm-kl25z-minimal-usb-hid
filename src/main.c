#include "MKL25Z4.h"
#include "gpio.h"
#include "usb_device.h"

volatile unsigned millitime = 0;

static void send_str(char* s) {
    while (*s) {
        fifo_push(&usb_tx, *s++);
    }
}

int main(void) {

    unsigned start_time = 0;
    uint8_t count = 0;
    uint8_t c;

    SysTick_Config(48000000/1000);
    gpio_init();
    usb_device_init();

    while(1) {
        asm("wfi");

        /*
         * pump everything from RX straight back into TX...
         */
        while(fifo_pop(&usb_rx, &c)) {
            fifo_push(&usb_tx, c);
        }

        /*
         * ...and insert a funny letter from time to time
         */
        if (millitime - start_time > 250) {
            start_time = millitime;
            fifo_push(&usb_tx, 65 + count);
            if (count++ == 25) {
                count = 0;
            }
        }
    }
}


/*
 * Hooks and Interrupts
 */

/**
 * Hook is called when receivig a message packet.
 * @param data pointer to 62 bytes containg the message
 */
void usb_hook_message_packet(volatile uint8_t* data) {
    if (data[0] == 1) {
        LED_BL_low();
        send_str("blue led has been turned on!\n");
    } else {
        LED_BL_high();
        send_str("blue led has been turned off!\n");
    }
}

/**
 * Hook is called to control the RX LED
 * @param on true if LED should be on, false otherwise
 */
void usb_hook_led_rx(bool on) {
    if (on) {
        LED_RD_low();
    } else {
        LED_RD_high();
    }
}

/**
 * Hook is called to control the RX LED
 * @param on true if LED should be on, false otherwise
 */
void usb_hook_led_tx(bool on) {
    if (on) {
        LED_GN_low();
    } else {
        LED_GN_high();
    }
}

void SysTick_Handler(void) {
    ++millitime;
}
