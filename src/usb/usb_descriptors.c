/* generated code, do not edit! */

#include "usb_descriptors.h"

static const uint8_t language_descriptor[4] = {
    0x04, // bLength
    0x03, // bDescriptorType
    0x09, // English
    0x04, // US
};

static const uint8_t string_descriptor_1[20] = {
    0x14, // bLength
    0x03, // bDescriptorType
    0x41, // UTF-16-LE: "A"
    0x00,
    0x43, // UTF-16-LE: "C"
    0x00,
    0x4d, // UTF-16-LE: "M"
    0x00,
    0x45, // UTF-16-LE: "E"
    0x00,
    0x20, // UTF-16-LE: " "
    0x00,
    0x49, // UTF-16-LE: "I"
    0x00,
    0x6e, // UTF-16-LE: "n"
    0x00,
    0x63, // UTF-16-LE: "c"
    0x00,
    0x2e, // UTF-16-LE: "."
    0x00,
};

static const uint8_t string_descriptor_2[24] = {
    0x18, // bLength
    0x03, // bDescriptorType
    0x44, // UTF-16-LE: "D"
    0x00,
    0x65, // UTF-16-LE: "e"
    0x00,
    0x6d, // UTF-16-LE: "m"
    0x00,
    0x6f, // UTF-16-LE: "o"
    0x00,
    0x20, // UTF-16-LE: " "
    0x00,
    0x44, // UTF-16-LE: "D"
    0x00,
    0x65, // UTF-16-LE: "e"
    0x00,
    0x76, // UTF-16-LE: "v"
    0x00,
    0x69, // UTF-16-LE: "i"
    0x00,
    0x63, // UTF-16-LE: "c"
    0x00,
    0x65, // UTF-16-LE: "e"
    0x00,
};

static const uint8_t string_descriptor_3[18] = {
    0x12, // bLength
    0x03, // bDescriptorType
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
    0x30, // UTF-16-LE: "0"
    0x00,
};

static const uint8_t device_descriptor[18] = {
    0x12, // bLength
    0x01, // bDescriptorType
    0x01, // bcdUSB (lo)
    0x01, // bcdUSB (hi)
    0x00, // bDeviceClass
    0x00, // bDeviceSubClass
    0x00, // bDeviceProtocol
    0x40, // bMaxPacketSize
    0xad, // idVendor (lo)
    0xde, // idVendor (hi)
    0xef, // idProduct (lo)
    0xbe, // idProduct (hi)
    0x00, // bcdDevice (lo)
    0x00, // bcdDevice (hi)
    0x01, // iManufacturer "ACME Inc."
    0x02, // iProduct "Demo Device"
    0x03, // iSerial "00000000"
    0x01, // bNumConfigurations
};

static const uint8_t report_descriptor[32] = {
    0x05,
    0x01,
    0x09,
    0x00,
    0xa1,
    0x01,
    0x15,
    0x00,
    0x26,
    0xff,
    0x00,
    0x85,
    0x01,
    0x75,
    0x08,
    0x95,
    0x3f,
    0x09,
    0x00,
    0x81,
    0x82,
    0x85,
    0x02,
    0x75,
    0x08,
    0x95,
    0x3f,
    0x09,
    0x00,
    0x91,
    0x82,
    0xc0,
};

static const uint8_t string_descriptor_4[32] = {
    0x20, // bLength
    0x03, // bDescriptorType
    0x53, // UTF-16-LE: "S"
    0x00,
    0x74, // UTF-16-LE: "t"
    0x00,
    0x72, // UTF-16-LE: "r"
    0x00,
    0x65, // UTF-16-LE: "e"
    0x00,
    0x61, // UTF-16-LE: "a"
    0x00,
    0x6d, // UTF-16-LE: "m"
    0x00,
    0x20, // UTF-16-LE: " "
    0x00,
    0x6f, // UTF-16-LE: "o"
    0x00,
    0x76, // UTF-16-LE: "v"
    0x00,
    0x65, // UTF-16-LE: "e"
    0x00,
    0x72, // UTF-16-LE: "r"
    0x00,
    0x20, // UTF-16-LE: " "
    0x00,
    0x48, // UTF-16-LE: "H"
    0x00,
    0x49, // UTF-16-LE: "I"
    0x00,
    0x44, // UTF-16-LE: "D"
    0x00,
};

static const uint8_t string_descriptor_5[44] = {
    0x2c, // bLength
    0x03, // bDescriptorType
    0x44, // UTF-16-LE: "D"
    0x00,
    0x65, // UTF-16-LE: "e"
    0x00,
    0x66, // UTF-16-LE: "f"
    0x00,
    0x61, // UTF-16-LE: "a"
    0x00,
    0x75, // UTF-16-LE: "u"
    0x00,
    0x6c, // UTF-16-LE: "l"
    0x00,
    0x74, // UTF-16-LE: "t"
    0x00,
    0x20, // UTF-16-LE: " "
    0x00,
    0x43, // UTF-16-LE: "C"
    0x00,
    0x6f, // UTF-16-LE: "o"
    0x00,
    0x6e, // UTF-16-LE: "n"
    0x00,
    0x66, // UTF-16-LE: "f"
    0x00,
    0x69, // UTF-16-LE: "i"
    0x00,
    0x67, // UTF-16-LE: "g"
    0x00,
    0x75, // UTF-16-LE: "u"
    0x00,
    0x72, // UTF-16-LE: "r"
    0x00,
    0x61, // UTF-16-LE: "a"
    0x00,
    0x74, // UTF-16-LE: "t"
    0x00,
    0x69, // UTF-16-LE: "i"
    0x00,
    0x6f, // UTF-16-LE: "o"
    0x00,
    0x6e, // UTF-16-LE: "n"
    0x00,
};

static const uint8_t configuration_descriptor[41] = {
    0x09, // bLength (*** Configuration ***
    0x02, // bDescriptorType
    0x29, // wTotalLength (lo)
    0x00, // wTotalLength (hi)
    0x01, // bNumInterfaces
    0x01, // bConfigurationValue
    0x05, // iConfiguration "Default Configuration"
    0x80, // bmAttributes
    0xfa, // bMaxPower
    0x09, // bLength (*** Interface ***
    0x04, // bDescriptorType
    0x00, // iInterfaceNumber
    0x00, // bAlternateSetting
    0x02, // bNumEndpoints
    0x03, // bInterfaceClass
    0x00, // bInterfaceSubClass
    0x00, // bInterfaceProtocol
    0x04, // iInterface "Stream over HID"
    0x09, // bLength (*** HID-Descriptor ***
    0x21, // bDescriptorType
    0x01, // bcdHid (lo)
    0x01, // bcdHid (hi)
    0x00, // bCountryCode
    0x01, // bNumDescriptors
    0x22, // bDescriptorType
    0x20, // wItemLength (lo)
    0x00, // wItemLength (hi)
    0x07, // bLength (*** Endpoint ***)
    0x05, // bDescriptorType
    0x81, // bEndpointAddr
    0x03, // bmAttributes
    0x40, // wMaxPacketSize (lo)
    0x00, // wMaxPacketSize (hi)
    0x01, // bInterval
    0x07, // bLength (*** Endpoint ***)
    0x05, // bDescriptorType
    0x01, // bEndpointAddr
    0x03, // bmAttributes
    0x40, // wMaxPacketSize (lo)
    0x00, // wMaxPacketSize (hi)
    0x01, // bInterval
};

const descriptor_table_t descriptor_table[9] ={
    {0x0300, 0x0000, &language_descriptor, sizeof(language_descriptor)},
    {0x0301, 0x0409, &string_descriptor_1, sizeof(string_descriptor_1)},
    {0x0302, 0x0409, &string_descriptor_2, sizeof(string_descriptor_2)},
    {0x0303, 0x0409, &string_descriptor_3, sizeof(string_descriptor_3)},
    {0x0100, 0x0000, &device_descriptor, sizeof(device_descriptor)},
    {0x2200, 0x0000, &report_descriptor, sizeof(report_descriptor)},
    {0x0304, 0x0409, &string_descriptor_4, sizeof(string_descriptor_4)},
    {0x0305, 0x0409, &string_descriptor_5, sizeof(string_descriptor_5)},
    {0x0200, 0x0000, &configuration_descriptor, sizeof(configuration_descriptor)},
};

