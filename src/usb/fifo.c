/*
 * fifo.c
 *
 *  Created on: 05.11.2016
 *      Author: bernd
 */

#include "fifo.h"

void fifo_init(fifo_t* self, uint8_t* buffer, unsigned capacity) {
    self->capacity = capacity;
    self->buffer = buffer;
    self->write_index = 0;
    self->read_index = 0;
}

unsigned fifo_get_size(fifo_t* self) {
    int s = self->write_index - self->read_index;
    if (s < 0) {
        s += self->capacity;
    }
    return s;
}

bool fifo_push(fifo_t* self, uint8_t byte) {
    if (fifo_get_size(self) < self->capacity) {
        unsigned i = self->write_index;
        self->buffer[i++] = byte;
        if (i == self->capacity) {
            i = 0;
        }
        self->write_index = i;
        return true;
    }
    return false;
}

bool fifo_pop(fifo_t* self, uint8_t* byte) {
    if (fifo_get_size(self)) {
        unsigned i = self->read_index;
        *byte = self->buffer[i++];
        if (i == self->capacity) {
            i = 0;
        }
        self->read_index = i;
        return true;
    }
    return false;
}


