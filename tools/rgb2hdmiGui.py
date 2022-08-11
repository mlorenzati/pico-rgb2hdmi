#!/Users/marcelo.lorenzati/.pyenv/shims/python3
from ctypes import alignment
import tkinter as tk
from tkinter import ANCHOR, FLAT, DISABLED, GROOVE, ttk
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
masterFrame.grid(row=0, padx=5, pady=5)

# Vertical Frames
upperFrame = ttk.Frame(masterFrame)
upperFrame.grid(column=0, row=0, padx=0, pady=0)

# On upper Frame, image and buttons
# Image
img = ImageTk.PhotoImage(Image.open("./rgb2hdmilogo.png"))
imgLabel = ttk.Label(upperFrame, image = img, borderwidth=0, justify='center')
imgLabel.grid(column=0, row=0, padx=5, pady=10)

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
def about():
    popup = tk.Tk()
    
    def leavemini():
        popup.destroy()

    popup.wm_title("About pico-RGB2HDMI APP")
    popup.wm_attributes('-topmost', True)     # keeps popup above everything until closed.
    popup.resizable(False, False)

    # this next label (tk.button) is the text field holding your message. i put it in a tk.button so the sizing matched the "close" button
    # also want to note that my button is very big due to it being used on a touch screen application.
    textStr = """RGB2HMI APP is the companion application for the converter,
to command it remotely through USB, list of support will increase over time
Thanks to the supporters of this project: Fernando Bugallo, Jose Francisco Manera,
Gaston Martin Ferreiros, Otto, Sebastian Rho, Gabriel Garcia, Alejandro Perez, Juan Pablo Chucair
Aldo Guillermo Ibañez and Emiliano Escobar

All rights reserved, pico-RGG2HMDI - Marcelo Lorenzati"""
    text=tk.Text(popup, width=100)
    text.insert(tk.END, textStr)
    text.config(state='disabled')
    text.pack(side="top")
    close_button = ttk.Button(popup, text="Close", command=leavemini, width=30)
    close_button.pack(side="top")

buttonFrame = ttk.Frame(upperFrame, width=200, height=600)
buttonFrame.grid(column=1, row=0, padx=0, pady=10)

cursorFrame = ttk.Frame(buttonFrame)
cursorFrame.grid(column=0, row=0, padx=0, pady=10)

buttonUp = ttk.Button(cursorFrame, text='Up', command=move_up)
buttonUp.grid(column=1, row=0, padx=0, pady=1)
buttonDown = ttk.Button(cursorFrame, text='Down', command=move_down)
buttonDown.grid(column=1, row=2, padx=0, pady=1)
buttonLeft= ttk.Button(cursorFrame, text='Left', command=move_up)
buttonLeft.grid(column=0, row=1, padx=0, pady=1)
buttonRight = ttk.Button(cursorFrame, text='Right', command=move_down)
buttonRight.grid(column=2, row=1, padx=0, pady=1)

infoLabel = ttk.Label(buttonFrame, text="Device ID:\nVersion:\nResolution:")
infoLabel.grid(column=0, row=1, padx=5, pady=10, sticky='w')

buttonCapture = ttk.Button(buttonFrame, text='Capture', command=capture)
buttonCapture.grid(column=0, row=2, padx=0, pady=10)

buttonSave = ttk.Button(buttonFrame, text='Save', command=save)
buttonSave.grid(column=0, row=3, padx=0, pady=10)

usbLabel = ttk.Label(buttonFrame, text="USB Port:")
usbLabel.grid(column=0, row=4, padx=5, pady=10, sticky='w')

buttonAbout = ttk.Button(buttonFrame, text='About', command=about)
buttonAbout.grid(column=0, row=5, padx=0, pady=10)

# On lower Frame, Status
#Status Bar
statusLabel= ScrolledText(master = masterFrame, width=125, height=10)
statusLabel.grid(row=1, padx=0, pady=0)
statusLabel.insert(tk.END, "RGB2HDMIAPP Started")
statusLabel.config(state='disabled')

def checkSerial():
    ports=serialCmd.get_serial_ports()
    print("serial port checked", ports)

root.after(100, checkSerial)
root.wm_attributes('-topmost', True) 
root.mainloop()