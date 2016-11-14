/*
 * usb_device.h
 *
 *  Created on: 02.11.2016
 *      Author: Bernd Kreuss
 */

#ifndef SRC_USB_USB_DEVICE_H_
#define SRC_USB_USB_DEVICE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb_descriptors.h"
#include "fifo.h"

void usb_device_init(void);

extern fifo_t usb_hidstream_tx;
extern fifo_t usb_hidstream_rx;

/*
 * The hooks below can be implemented by the application,
 * they are optional and default to weak empty stubs.
 */
extern void usb_hook_led_tx(bool on);
extern void usb_hook_led_rx(bool on);
extern void usb_hook_message_packet(volatile uint8_t* data);

#endif /* SRC_USB_USB_DEVICE_H_ */
