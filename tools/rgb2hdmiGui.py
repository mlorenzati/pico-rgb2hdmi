#!/Users/marcelo.lorenzati/.pyenv/shims/python3
from ctypes import alignment
import tkinter as tk
from tkinter import ANCHOR, ttk
from tkinter . scrolledtext import ScrolledText
from PIL import ImageTk, Image
import serialCmd as serialCmd

# root window
root = tk.Tk()

# configure the root window
root.geometry("")
root.resizable(False, False)
root.title('pico-rgb2hdmi App')

# label frame
masterFrame = ttk.Frame(root)
masterFrame.grid(row=0, padx=0, pady=0)

# Vertical Frames
upperFrame = ttk.Frame(masterFrame)
upperFrame.grid(column=0, row=0, padx=0, pady=0)

# On upper Frame, image and buttons
# Image
img = ImageTk.PhotoImage(Image.open("./rgb2hdmilogo.png"))
imgLabel = ttk.Label(upperFrame, image = img, borderwidth=0, justify='center')
imgLabel.grid(column=0, row=0, padx=4, pady=4)

# Button Group
def move_up():
    print("up")
def move_down():
    print("down")
def move_left():
    print("left")
def move_right():
    print("right")
def capture():
    print("capture")
def save():
    print("save")
buttonFrame = ttk.Frame(upperFrame, width=200, height=600)
buttonFrame.grid(column=1, row=0, padx=0, pady=5, sticky='n')

cursorFrame = ttk.Frame(buttonFrame)
cursorFrame.grid(column=0, row=0, padx=0, pady=5)

buttonUp = ttk.Button(cursorFrame, text='Up', command=move_up)
buttonUp.grid(column=1, row=0, padx=0, pady=1)
buttonDown = ttk.Button(cursorFrame, text='Down', command=move_down)
buttonDown.grid(column=1, row=2, padx=0, pady=1)
buttonLeft= ttk.Button(cursorFrame, text='Left', command=move_up)
buttonLeft.grid(column=0, row=1, padx=0, pady=1)
buttonRight = ttk.Button(cursorFrame, text='Right', command=move_down)
buttonRight.grid(column=2, row=1, padx=0, pady=1)

infoLabel = ttk.Label(buttonFrame, text="Device ID:\nVersion:\nResolution:")
infoLabel.grid(column=0, row=1, padx=5, pady=5, sticky='w')

buttonCapture = ttk.Button(buttonFrame, text='Capture', command=capture, width=26)
buttonCapture.grid(column=0, row=2, padx=0, pady=5)

buttonSave = ttk.Button(buttonFrame, text='Save', command=save, width=26)
buttonSave.grid(column=0, row=3, padx=0, pady=5)

usbLabel = ttk.Label(buttonFrame, text="USB Port:")
usbLabel.grid(column=0, row=4, padx=5, pady=5, sticky='w')

# On lower Frame, Status
#Status Bar
statusLabel= ScrolledText(master = masterFrame, width=125, height=10)
statusLabel.grid(row=1, padx=0, pady=0)
statusLabel.insert(tk.END, "RGB2HDMIAPP Started" )
statusLabel.config(state='disabled')

def checkSerial():
    ports=serialCmd.get_serial_ports()
    print("serial port checked", ports)

root.after(100, checkSerial)
root.mainloop()