#!/Users/marcelo.lorenzati/.pyenv/shims/python3
import numpy as np
from PIL import Image
import sys
from io import StringIO

def processRGBFromStrArray(str):
    arrayStr = (str.split("\n",1)[1]).replace("\r","")
    return processRGBFromFile(StringIO(arrayStr))

def processRGBFromFile(file):
    firstArray = (file.readline()).split(',')
    width = len(firstArray)
    wordSize = len(firstArray[0]) * 4
    rgbArray = np.loadtxt(file, delimiter = ',', dtype='int16', converters = {_:lambda s: int(s, 16) for _ in range(width)})
    return processArrayToRGBImage(rgbArray, wordSize)

def processArrayToRGBImage(array, wordSize):
    # Pick up image dimensions
    h, w = array.shape

    # Report format
    print("Image is", w, "*", h+1, " with", wordSize, "bits color density")

    # Make a numpy array of matching shape, but allowing for 8-bit/channel for R, G and B
    rgb888array = np.zeros([h,w,3], dtype=np.uint8)
   
    for row in range(h):
        for col in range(w):
            # Pick up rgbPixel value and split into rgb888
            rgbPixel = array[row,col]
            if  (wordSize == 16):
                r = ((rgbPixel >> 11 ) & 0x1f ) << 3
                g = ((rgbPixel >> 5  ) & 0x3f ) << 2
                b = ((rgbPixel       ) & 0x1f ) << 3
            elif (wordSize == 8):
                r = ((rgbPixel >> 5  ) & 0x7 ) << 5
                g = ((rgbPixel >> 2  ) & 0x7 ) << 5
                b = ((rgbPixel       ) & 0x3 ) << 6
            else:
                print('input formatting error, neither 8 or 16 bits hexa, comma separated')
                return None
            # Populate result array
            rgb888array[row,col]=r,g,b
           
    aspectRatio43Height=int((w*3)/4)
    return Image.fromarray(rgb888array).resize((w, aspectRatio43Height), resample=Image.NEAREST)

def runCmd():
    if len(sys.argv)!=2:
        print('Usage: ', sys.argv[0], 'file.csv')
        exit(1)
    # Read 16-bit rgbPixel / 8bit RGb332 image into array of uint
    filame = sys.argv[1]
    try:
        with open(filame,'r') as file:
            image = processRGBFromFile(file)
            if image == None:
                exit(1)
            # Save result as PNG
            outputFile=sys.argv[1].rsplit('.')[0] + '.png'
            image.save(outputFile)
    except Exception as e: 
        print('file error:', e)
        exit(1)

if sys.argv[0] == "./csv2png.py" or sys.argv[0] == "csv2png.py":
    runCmd()