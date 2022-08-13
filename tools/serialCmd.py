#!/Users/marcelo.lorenzati/.pyenv/shims/python3
import sys
import glob
import time
import serial
import re

def get_serial_ports():
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

printed_skipped_just_once=False
def get_rgb_2_hdmi_ports():
    global printed_skipped_just_once
    invalid_portnames = [ 'bluetooth']
    ports=get_serial_ports()
    
    for port in ports:
        skip=False
        for invalid_port in invalid_portnames:
            if re.search(invalid_port, port, re.IGNORECASE):
                if not printed_skipped_just_once:
                    print('Skip port', port)
                    printed_skipped_just_once=True
                skip=True
                break
        if skip:
            continue
        ser = serial.Serial(
            port=port,
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )
        if not ser.isOpen():
            continue
        print("Testing port", port)

        res = sendCommand(ser, "version")
        if res[0]:
            print("Found device on ", port, "(", res[1], ")")
            return (ser, port, res[1])
    return None
    
def sendCommand(ser, val):
    ser.write((val +'\n').encode('ascii'))
    ackLine=ser.readline()
    args=val.split(' ', 1)
    cmd = args[0]
    val = ""
    if len(args)>1:
        val=args[1]
    expected=('Request '+ cmd + '<' + cmd[0] + '>(' + val + ')\r\n').encode('ascii').lower()
    if (ackLine.lower() != expected):
        ser.flushInput()
        return (False, '')
    
    partial = ''
    result = ''
    while ( (partial := ser.readline()) != b''):
        result = result + partial.decode('ascii')

    return (True, result.rstrip())