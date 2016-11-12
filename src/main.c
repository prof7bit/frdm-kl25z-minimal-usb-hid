#include "MKL25Z4.h"
#include "gpio.h"
#include "usb_device.h"

volatile unsigned millitime = 0;

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
        while(fifo_pop_front(&usb_hidstream_rx, &c)) {
            fifo_push_back(&usb_hidstream_tx, c);
        }

        /*
         * ...and insert a funny letter from time to time
         */
        if (millitime - start_time > 250) {
            start_time = millitime;
            fifo_push_back(&usb_hidstream_tx, 65 + count);
            if (count++ == 25) {
                count = 0;
            }
        }
    }
}

/*
 * Implement the hooks for the USB RX/TX LEDs
 */

void usb_hook_led_rx(bool on) {
    if (on) {
        LED_RD_low();
    } else {
        LED_RD_high();
    }
}

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
