from datetime import *
from threading import *
import sys, os, time
import tkinter as tk
import serialCmd as serialCmd
import csv2png as csv2png
from tkinter import ANCHOR, FLAT, DISABLED, NORMAL, GROOVE, ttk
from tkinter . scrolledtext import ScrolledText
from PIL import ImageTk, Image
from tkinter.filedialog import asksaveasfile
import signal

os.environ['TK_SILENCE_DEPRECATION'] = '1'

class StdoutRedirector(object):
    def __init__(self,text_widget):
        self.text_space = text_widget

    def write(self,string):
        self.text_space.config(state='normal')
        self.text_space.insert('end', string)
        self.text_space.see('end')
        self.text_space.config(state='disabled')
    
    def flush(self):
        pass

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
saveImg = Image.open("./rgb2hdmilogo.png")
frontImg = ImageTk.PhotoImage(saveImg)
imgLabel = ttk.Label(upperFrame, image = frontImg, borderwidth=0, justify='center')
imgLabel.grid(column=0, row=0, padx=5, pady=10)

# Button Group
def move_up(step):
    res = serialCmd.sendCommand(serial_port, "up " + step)
    print("Move up",    step, ("step", "steps")[int(step)>1], ("fail", "success")[res[0]])
def move_down(step):
    res = serialCmd.sendCommand(serial_port, "down " + step)
    print("Move down",  step, ("step", "steps")[int(step)>1], ("fail", "success")[res[0]])
def move_left(step):
    res = serialCmd.sendCommand(serial_port, "left " + step)
    print("Move left",  step, ("step", "steps")[int(step)>1], ("fail", "success")[res[0]])
def move_right(step):
    res = serialCmd.sendCommand(serial_port, "right " + step)
    print("Move right", step,("step", "steps")[int(step)>1],  ("fail", "success")[res[0]])

def capture_in_thread():
    captureThread=Thread(target=capture)
    captureThread.start()

def capture():
    global buttonCapture
    disableButtons(True)
    print("Starting capture")
    res = serialCmd.sendCommand(serial_port, "capture")
    if res[0]:
        global frontImg
        global imgLabel
        global saveImg
        print("Capture success")
        saveImg = csv2png.processRGBFromStrArray(res[1])
        img = saveImg.resize((640,480))
        frontImg = ImageTk.PhotoImage(image=img)
        imgLabel.config(image=frontImg)
    else:
        print("Capture fail")
    disableButtons(False)
    
def save():
    global saveImg
    print("Open save window")
    now=datetime.now().strftime("%Y_%m_%d-%H_%M_%S")
    initialFilename = 'rbg2hdmi-'+rgb2hdmi_deviceId+'-'+now+'.png'
    file = asksaveasfile(initialfile = initialFilename, defaultextension=".png", filetypes=[("Image Files","*.png")])
    if file:
        saveImg.save(os.path.abspath(file.name))

def about():
    popup = tk.Tk()
    popup.grab_set()
    popup.geometry("")
    
    def leavemini():
        popup.grab_release()
        popup.destroy()

    popup.wm_title("About pico-RGB2HDMI APP")
    popup.wm_attributes('-topmost', True)     # keeps popup above everything until closed.
    popup.resizable(False, False)

    # this next label (tk.button) is the text field holding your message. i put it in a tk.button so the sizing matched the "close" button
    # also want to note that my button is very big due to it being used on a touch screen application.
    textStr = """RGB2HMI APP is the companion application for the converter, to command it remotely through USB, 
list of support will increase over time.
USB is not activated by default on the pico-RGB2HDMI, you need to press the 3 buttons at the same time to activate.

Thanks to the supporters of this project: Fernando Bugallo, Jose Francisco Manera, Gaston Martin Ferreiros, 
Otto, Sebastian Rho, Gabriel Garcia, Alejandro Perez, Juan Pablo Chucair
Aldo Guillermo Ibañez and Emiliano Escobar

All rights reserved, pico-RGG2HMDI - Marcelo Lorenzati
mlorenzati@gmail.com"""
    text=tk.Text(popup, width=98, font=("Arial", 15), height=10, padx=4)
    text.insert(tk.END, textStr)
    text.config(state='disabled')
    text.pack(side="top")
    close_button = ttk.Button(popup, text="Close", command=leavemini, width=30)
    close_button.pack(side="top")

buttonFrame = ttk.Frame(upperFrame, width=200, height=600)
buttonFrame.grid(column=1, row=0, padx=0, pady=10)

stepFrame = ttk.Frame(buttonFrame)
stepFrame.grid(column=0, row=0, padx=0, pady=2)
stepLabel = ttk.Label(stepFrame, text="Steps")
stepLabel.grid(column=0, row=0, padx=0, pady=2)
stepInput = ttk.Spinbox(stepFrame, width=5, from_=1, to_=20)
stepInput.set(1)
stepInput.grid(column=1, row=0, padx=0, pady=2)

cursorFrame = ttk.Frame(buttonFrame)
cursorFrame.grid(column=0, row=1, padx=0, pady=10)

buttonUp = ttk.Button(cursorFrame, text='Up', command = lambda: move_up(stepInput.get()))
buttonUp.grid(column=1, row=0, padx=0, pady=1)
buttonDown = ttk.Button(cursorFrame, text='Down', command = lambda: move_down(stepInput.get()))
buttonDown.grid(column=1, row=2, padx=0, pady=1)
buttonLeft= ttk.Button(cursorFrame, text='Left', command = lambda: move_left(stepInput.get()))
buttonLeft.grid(column=0, row=1, padx=0, pady=1)
buttonRight = ttk.Button(cursorFrame, text='Right', command = lambda: move_right(stepInput.get()))
buttonRight.grid(column=2, row=1, padx=0, pady=1)


rgb2hdmi_deviceId='...'
rgb2hdmi_version='...'
rgb2hdmi_resolution='...'
def setDeviceVersion(ver):
    global rgb2hdmi_version
    rgb2hdmi_version = ver
def setDeviceId(id):
    global rgb2hdmi_deviceId
    rgb2hdmi_deviceId= id
def setDeviceResolution(res):
    global rgb2hdmi_resolution
    rgb2hdmi_resolution = res

def getStatusInfo():
    return "Device ID: " + rgb2hdmi_deviceId + "\nVersion: " + rgb2hdmi_version + "\nResolution: " + rgb2hdmi_resolution

def setStatusLabel():
    infoLabel.config(text=getStatusInfo())
    infoLabel.update()

infoLabel = ttk.Label(buttonFrame, text=getStatusInfo())
infoLabel.grid(column=0, row=2, padx=5, pady=10, sticky='w')

buttonCapture = ttk.Button(buttonFrame, text='Capture', command=capture_in_thread)
buttonCapture.grid(column=0, row=3, padx=0, pady=10)

buttonSave = ttk.Button(buttonFrame, text='Save', command=save)
buttonSave.grid(column=0, row=4, padx=0, pady=10)

usbPreText = "USB Port:"
usbLabel = ttk.Label(buttonFrame, text=usbPreText)
usbLabel.grid(column=0, row=5, padx=5, pady=10, sticky='w')

buttonAbout = ttk.Button(buttonFrame, text='About', command=about)
buttonAbout.grid(column=0, row=6, padx=0, pady=10)

# On lower Frame, Status
#Status Bar
statusLabel= ScrolledText(master = masterFrame, width=125, height=10)
statusLabel.grid(row=1, padx=0, pady=0)
statusLabel.config(state='disabled')
sys.stdout = StdoutRedirector(statusLabel)

def disableButtons(val):
    buttons = [buttonUp, buttonDown, buttonLeft, buttonRight, buttonCapture]
    state = (NORMAL, DISABLED)[val]
    for button in buttons:
        button['state'] = state

serial_thread_running=True
def on_closing():
    sys.stdout = sys.__stdout__
    print("Closing window")
    global serial_thread_running
    global serialThread
    serial_thread_running = False
    serialThread = None

    root.destroy()

serial_port = None
def checkSerial():
    while (serial_thread_running):
        disableButtons(True)
        root.wm_attributes('-topmost', False) 
        global serial_port
        while True:
            port=serialCmd.get_rgb_2_hdmi_ports()
            if port == None:
                usbLabel.config(text=usbPreText)
            else:
                break
        serial_port = port[0]
        usbLabel.config(text=usbPreText + port[1])
        usbLabel.update()
        setDeviceVersion(port[2].split("version ",1)[1])
        res = serialCmd.sendCommand(serial_port, "id")
        if res[0]:
            setDeviceId(res[1].split("Device is: ",1)[1])
        res = serialCmd.sendCommand(serial_port, "mode")
        if res[0]:
            setDeviceResolution(res[1].split("pico-rgb2hdmi ",1)[1])
        setStatusLabel()
        disableButtons(False)
        while (serialCmd.isConnected(serial_port) and serial_thread_running):
            time.sleep(1)
        print("Detected disconnection")

def handler(signum, frame):
    global serial_thread_running
    global serialThread
    sys.stdout = sys.__stdout__
    print("Closing cmd")
    serial_thread_running = False
    serialThread = None

    root.destroy()

print("RGB2HDMIAPP Started")
signal.signal(signal.SIGINT, handler)

serialThread=Thread(target=checkSerial)
root.after(100, lambda: (serialThread.start()))
root.wm_attributes('-topmost', True)
root.protocol("WM_DELETE_WINDOW", on_closing)
root.mainloop()