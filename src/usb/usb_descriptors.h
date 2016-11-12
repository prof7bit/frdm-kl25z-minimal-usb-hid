/* generated code, do not edit! */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>

typedef struct {
    uint16_t wValue;
    uint16_t wIndex;
    const void* descriptor;
    uint16_t size;
} descriptor_table_t;

extern const descriptor_table_t descriptor_table[9];

#endif
