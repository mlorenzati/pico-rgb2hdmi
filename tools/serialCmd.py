#!/Users/marcelo.lorenzati/.pyenv/shims/python3
import sys
import glob
import time
import serial


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

def wait_for_newline(ser):
    result = ''
    while True:
        char = ser.read(1).decode('utf-8')
        print(char)
        if char == '\n':
            return result
        result=result + char

def get_rgb_2_hdmi_ports():
    ports=get_serial_ports()
    for port in ports:
        ser = serial.Serial(
            port=port,
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=0.2
        )
        if not ser.isOpen():
            continue
        ser.write('version\n'.encode())
        # response=ser.read(40).decode('utf-8')
        response=wait_for_newline(ser)
        print(response)

get_rgb_2_hdmi_ports()