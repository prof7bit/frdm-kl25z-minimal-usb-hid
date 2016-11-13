#!/usr/bin/python3

# this requires cython-hidapi:
# pip3 install hidapi
import hid

REPORT_ID_OUT = 2

def send_string(d, text):
    
    # truncate to 62 bytes
    text = text[:62]
    
    # prepare header
    x = [REPORT_ID_OUT, len(text)]
    for c in text:
        x.append(ord(c))
        
    # pad with zero bytes to make
    # bugged windows HID driver happy
    while len(x) < 64:
        x.append(0)
    
    print("    sent: " + text)
    d.write(x)

def recv_string(d):
    s = ""
    x = d.read(64)
    if len(x) > 2:
        
        # parse header and
        # extract payload
        size = x[1]
        x = x[2:][:size]
        
        # convert to string
        for c in x:
            s += chr(c)
            
    return s

def main():
    d = hid.device()
    d.open(0xdead, 0xbeef)

    for i in range(100):
        if i % 10 == 0:
            send_string(d, "Hello world!")
        print("received: " + recv_string(d))

    d.close()

if __name__ == '__main__':
    main()
