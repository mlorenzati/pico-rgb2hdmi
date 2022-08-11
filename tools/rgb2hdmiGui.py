#!/Users/marcelo.lorenzati/.pyenv/shims/python3
from ctypes import alignment
import tkinter as tk
from tkinter import ttk
from tkinter . scrolledtext import ScrolledText
from PIL import ImageTk, Image
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
img = ImageTk.PhotoImage(Image.open("./file_640_240_8.png"))
imgLabel = ttk.Label(upperFrame, image = img, borderwidth=0)
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
buttonFrame = ttk.Frame(upperFrame, width=200, height=600)
buttonFrame.grid(column=1, row=0, padx=4, pady=4)

buttonUp = ttk.Button(buttonFrame, text='Up', command=move_up)
buttonUp.grid(column=1, row=0, ipadx=1, ipady=1)
buttonDown = ttk.Button(buttonFrame, text='Down', command=move_down)
buttonDown.grid(column=1, row=2, ipadx=1, ipady=1)
buttonLeft= ttk.Button(buttonFrame, text='Left', command=move_up)
buttonLeft.grid(column=0, row=1, ipadx=1, ipady=1)
buttonRight = ttk.Button(buttonFrame, text='Right', command=move_down)
buttonRight.grid(column=2, row=1, ipadx=1, ipady=1)

# On lower Frame, Status
#Status Bar
statusLabel= ScrolledText(master = masterFrame, width=125, height=10)
statusLabel.config(state='disabled')
statusLabel.grid(row=1, ipadx=0, ipady=0)

root.mainloop()