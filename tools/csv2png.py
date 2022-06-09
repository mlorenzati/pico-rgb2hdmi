#!/Users/marcelo.lorenzati/.pyenv/shims/python3

from pathlib import Path
import sys
import numpy as np
from PIL import Image

if len(sys.argv)!=2:
    print('Usage: ', sys.argv[0], 'file.csv')
    exit(1)

# Read 16-bit RGB565 image into array of uint16
try:
    with open(sys.argv[1],'r') as file:
        rgb565array = np.genfromtxt(file, delimiter = ',').astype(np.uint16)
except Exception as e: 
    print('file error ', e)
    exit(1)
# Pick up image dimensions
h, w = rgb565array.shape

# Make a numpy array of matching shape, but allowing for 8-bit/channel for R, G and B
rgb888array = np.zeros([h,w,3], dtype=np.uint8)

for row in range(h):
    for col in range(w):
        # Pick up rgb565 value and split into rgb888
        rgb565 = rgb565array[row,col]
        r = ((rgb565 >> 11 ) & 0x1f ) << 3
        g = ((rgb565 >> 5  ) & 0x3f ) << 2
        b = ((rgb565       ) & 0x1f ) << 3
        # Populate result array
        rgb888array[row,col]=r,g,b

# Save result as PNG
outputFile=sys.argv[1].rsplit('.')[0] + '.png'
Image.fromarray(rgb888array).save(outputFile)