#!/usr/bin/python3 -u

# this requires cython-hidapi:
# pip3 install hidapi
import hid
import os

def send_hid(d, x):
    if os.name == 'nt':
        buf = [0] + x
    else:
        buf = x
    return d.write(buf)


def send_string(d, text):
    
    # truncate to 63 bytes
    text = text[:63]
    
    # prepare header
    x = [len(text)]
    for c in text:
        x.append(ord(c))
        
    # pad with zero bytes to make
    # bugged windows HID driver happy
    while len(x) < 64:
        x.append(0)
    
    num = send_hid(d, x)
    print("    sent: " + text)

def recv_string(d):
    s = ""
    x = d.read(64)
    if len(x) > 1:
        
        # parse header and
        # extract payload
        size = x[0]
        x = x[1:][:size]
        
        # convert to string
        for c in x:
            s += chr(c)
            
    return s

def send_msg(d, b):
    x = [0] * 64
    x[0] = 255
    x[1] = b
    send_hid(d, x)

def main():
    toggle = 1
    d = hid.device()
    d.open(0xdead, 0xbeef)

    for i in range(100):
        if i % 10 == 0:
            send_string(d, "Hello world!")
        if i % 11 == 0:
            send_msg(d, toggle)
            toggle = 1 - toggle
        print("received: " + recv_string(d).strip())

    d.close()

if __name__ == '__main__':
    main()
