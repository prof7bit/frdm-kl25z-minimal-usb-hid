#!/usr/bin/python3

'''
Created on 11.11.2016

@author: Bernd Kreuss
'''


def generate(f):
    gen_device_descriptor(
        f,                  # file handle
        0xdead,             # idVendor
        0xbeef,             # idProduct
        0x0000,             # bcdDevice
        "ACME Inc.",        # vendor name
        "Demo Device",      # device name
        "00000000"          # serial number (place holder)
    )

    gen_report_descriptor(
        f,
        0x05, 0x01,         # USAGE_PAGE (Generic Desktop)
        0x09, 0x00,         # USAGE (Undefined)
        0xa1, 0x01,         # COLLECTION (Application)
        0x15, 0x00,         # LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00,   # LOGICAL_MAXIMUM (255)
        0x75, 0x08,         # REPORT_SIZE (8)
        0x95, 0x40,         # REPORT_COUNT (64)
        0x09, 0x00,         # USAGE (Undefined)
        0x81, 0x82,         # INPUT (Data,Var,Abs,Vol) - to the host
        0x75, 0x08,         # REPORT_SIZE (8)
        0x95, 0x40,         # REPORT_COUNT (64)
        0x09, 0x00,         # USAGE (Undefined)
        0x91, 0x82,         # OUTPUT (Data,Var,Abs,Vol) - from the host
        0xc0                # END_COLLECTION
    )

    gen_config_descriptor(
        f,                          # file handle
        "Default Configuration",    # iConfiguration
        0x80,                       # bmAttributes
        250,                        # bMaxPower
        interface(
            0,                      # iInterfaceNumber
            3,                      # bInterfaceClass
            0,                      # bInterfaceSubClass
            0,                      # bInterfaceProtocol
            "Stream over HID",      # iInterface

            hid(),                  # insert a HID descriptor here

            endpoint(
                0x81,               # bEndpointAddr
                0x03,               # bmAttributes
                64,                 # wMaxPacketSize
                1                   # bInterval
            ),

            endpoint(
                0x01,               # bEndpointAddr
                0x03,               # bmAttributes
                64,                 # wMaxPacketSize
                1                   # bInterval
            ),
        )
    )

    gen_descriptor_table(f)


#
# Leave stuff below here unchanged
#

import os
import sys

STRING_INDEX = 0
REP_DESC_LEN = 0
DESCR_TBL = []


def interface(iInterfaceNumber, bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol, sInterface, *args):
    bNumEndpoints = 0
    for arg in args:
        if arg[1][0] == 5:
            bNumEndpoints += 1
    members = [(9, "bLength (*** Interface ***"),
               (4, "bDescriptorType"),
               (iInterfaceNumber, "iInterfaceNumber"),
               (0, "bAlternateSetting"),
               (bNumEndpoints, "bNumEndpoints"),
               (bInterfaceClass, "bInterfaceClass"),
               (bInterfaceSubClass, "bInterfaceSubClass"),
               (bInterfaceProtocol, "bInterfaceProtocol"),
               make_string(sInterface, "iInterface")]
    for arg in args:
        members.extend(arg)
    return members


def endpoint(bEndpointAddr, bmAttributes, wMaxPacketSize, bInterval):
    members = [(7, "bLength (*** Endpoint ***)"),
               (5, "bDescriptorType"),
               (bEndpointAddr, "bEndpointAddr"),
               (bmAttributes, "bmAttributes"),
               (lo(wMaxPacketSize), "wMaxPacketSize (lo)"),
               (hi(wMaxPacketSize), "wMaxPacketSize (hi)"),
               (bInterval, "bInterval")]
    return members


def hid():
    global REP_DESC_LEN
    bcdHid = 0x0101
    members = [(9, "bLength (*** HID-Descriptor ***"),
               (0x21, "bDescriptorType"),
               (lo(bcdHid), "bcdHid (lo)"),
               (hi(bcdHid), "bcdHid (hi)"),
               (0, "bCountryCode"),
               (1, "bNumDescriptors"),
               (0x22, "bDescriptorType"),
               (lo(REP_DESC_LEN), "wItemLength (lo)"),
               (hi(REP_DESC_LEN), "wItemLength (hi)")]
    return members


def gen_config_descriptor(f, sConfiguration, bmAttributes, bMaxPower, *args):
    wTotalLength = 9
    bNumInterfaces = 0
    for arg in args:
        wTotalLength += len(arg)
        if arg[1][0] == 4:
            bNumInterfaces += 1

    members = [(9, "bLength (*** Configuration ***"),
               (2, "bDescriptorType"),
               (lo(wTotalLength), "wTotalLength (lo)"),
               (hi(wTotalLength), "wTotalLength (hi)"),
               (bNumInterfaces, "bNumInterfaces"),
               (1, "bConfigurationValue"),
               make_string(sConfiguration, "iConfiguration"),
               (bmAttributes, "bmAttributes"),
               (bMaxPower, "bMaxPower")]
    for arg in args:
        members.extend(arg)
    gen_array(f, "configuration_descriptor", members)
    DESCR_TBL.append((0x200, 0, "configuration_descriptor"))


def gen_report_descriptor(f, *args):
    global REP_DESC_LEN
    members = []
    for arg in args:
        members.append((arg, ""))
    gen_array(f, "report_descriptor", members)
    DESCR_TBL.append((0x2200, 0, "report_descriptor"))
    REP_DESC_LEN = len(members)


def gen_device_descriptor(f, idVendor, idProduct, bcdDevice, sManufacturer, sProduct, sSerial):
    bcdUSB = 0x0101
    members = [(18, "bLength"),
               (1, "bDescriptorType"),
               (lo(bcdUSB), 'bcdUSB (lo)'),
               (hi(bcdUSB), 'bcdUSB (hi)'),
               (0, 'bDeviceClass'),
               (0, 'bDeviceSubClass'),
               (0, 'bDeviceProtocol'),
               (64, 'bMaxPacketSize'),
               (lo(idVendor), 'idVendor (lo)'),
               (hi(idVendor), 'idVendor (hi)'),
               (lo(idProduct), 'idProduct (lo)'),
               (hi(idProduct), 'idProduct (hi)'),
               (lo(bcdDevice), 'bcdDevice (lo)'),
               (hi(bcdDevice), 'bcdDevice (hi)'),
               make_string(sManufacturer, 'iManufacturer'),
               make_string(sProduct, 'iProduct'),
               make_string(sSerial, 'iSerial'),
               (1, 'bNumConfigurations')]
    gen_array(f, "device_descriptor", members)
    DESCR_TBL.append((0x100, 0, "device_descriptor"))


def gen_string_descriptor(f, value):
    global STRING_INDEX
    if STRING_INDEX == 0:
        gen_lang_descriptor(f)
    members = [(3, "bDescriptorType")]
    for c in value:
        members.append((ord(c), "UTF-16-LE: \"{}\"".format(c)))
        members.append((0, ''))
    l = len(members) + 1
    members = [(l, "bLength")] + members
    gen_array(f, "string_descriptor_{}".format(STRING_INDEX), members)
    DESCR_TBL.append((0x300 + STRING_INDEX, 0x0409, "string_descriptor_{}".format(STRING_INDEX)))
    STRING_INDEX += 1


def gen_lang_descriptor(f):
    global STRING_INDEX
    members = [(4, "bLength"),
               (3, "bDescriptorType"),
               (0x09, "English"),
               (0x04, "US")]
    gen_array(f, "language_descriptor", members)
    DESCR_TBL.append((0x300, 0, "language_descriptor"))
    STRING_INDEX = 1


def gen_descriptor_table(f):
    f.write("const descriptor_table_t descriptor_table[{}] ={{\n".format(len(DESCR_TBL)))
    for item in DESCR_TBL:
        (wValue, wIndex, name) = item
        f.write("    {{0x{:04x}, 0x{:04x}, &{}, sizeof({})}},\n".format(wValue, wIndex, name, name))
    f.write("};\n\n")


#
# helper functions
#

def make_string(string, comment):
    if string != "":
        gen_string_descriptor(f, string)
        return (STRING_INDEX - 1, "{} \"{}\"".format(comment, string))
    else:
        return (0, comment + "{} (0 = empty)".format(comment))


def gen_array(f, name, members):
    f.write("static const uint8_t {}[{}] = {{\n".format(name, len(members)))
    for member in members:
        (value, comment) = member
        if comment != "":
            f.write("    0x{:02x}, // {}\n".format(value, comment))
        else:
            f.write("    0x{:02x},\n".format(value))
    f.write("};\n\n")


def gen_header_file():
    f = open(get_filename(".h"), "w")
    f.write("/* generated code, do not edit! */\n\n")
    f.write("#ifndef USB_DESCRIPTORS_H\n")
    f.write("#define USB_DESCRIPTORS_H\n\n")

    f.write("#include <stdint.h>\n\n")

    f.write("typedef struct {\n")
    f.write("    uint16_t wValue;\n")
    f.write("    uint16_t wIndex;\n")
    f.write("    const void* descriptor;\n")
    f.write("    uint16_t size;\n")
    f.write("} descriptor_table_t;\n\n")

    f.write("extern const descriptor_table_t descriptor_table[{}];\n\n".format(len(DESCR_TBL)))

    f.write("#endif\n")
    f.close()


def lo(word):
    return word & 0xff


def hi(word):
    return (word >> 8) & 0xff


def get_filename(ext):
    return os.path.join(sys.path[0], get_basename()) + ext


def get_basename():
    return "usb_descriptors"


if __name__ == '__main__':
    if sys.version_info[0] < 3:
        print("ERROR: Need Python-3.x, you have Python-{}.{}".format(sys.version_info[0], sys.version_info[1]))
        exit(1)

    print("generating USB descriptors...")
    f = open(get_filename(".c"), "w")
    f.write("/* generated code, do not edit! */\n\n")
    f.write("#include \"{}.h\"\n\n".format(get_basename()))
    generate(f)
    f.close()

    gen_header_file()
    print("generating USB descriptors done.")
