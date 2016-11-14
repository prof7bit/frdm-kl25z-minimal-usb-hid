/*
 * fifo.h
 *
 *  Created on: 05.11.2016
 *      Author: bernd
 */

#ifndef SRC_USB_FIFO_H_
#define SRC_USB_FIFO_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    volatile unsigned read_index;
    volatile unsigned write_index;
    volatile unsigned capacity;
    volatile uint8_t* buffer;
} fifo_t;


void fifo_init(fifo_t* self, uint8_t* buffer, unsigned capacity);
unsigned fifo_get_size(fifo_t* self);
bool fifo_push(fifo_t* self, uint8_t byte);
bool fifo_pop(fifo_t* self, uint8_t* byte);


#endif /* SRC_USB_FIFO_H_ */
