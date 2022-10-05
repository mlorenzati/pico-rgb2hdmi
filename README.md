# pico-rgb2hdmi
Raspberry Pico as an RGB Scanner to HDMI Converter

# Setup
- Download the pico-sdk, pico-extras, and pico-playground repositories
- Define PICO_SDK_PATH in your ~/.bashrc• 
- Download the pico-sdk repository

# Security
Install sha3sum to hash the binary

## Mac
```
brew install md5sha1sum
```

## Local sha3sum usage for sha256
```
sha1sum <file>
echo -n "<device_salt><device_uid>" | sha1sum -b
```

## converting CSV captures from serial port to images
```
python3 csv2png.py image.csv
```
This generates as output image.png

## Using packtiles to generate include headers from images

./packtiles -s -t 768 -f r1 char_c64_lineal.png font_c64.h

# Build
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
Go to the specific folder and

`make -j4`

# Visual Studio Code integration
Install the extensions as explained in the  [Pico Getting started manual](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)

Or just download [VScode](https://code.visualstudio.com/Download) and add the following extension

```
code --install-extension marus25.cortex-debug
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode.cpptools
```

# Debug
Install open ocd as explained in the [Pico Getting started manual section Installing OpenOCD](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)

#Using Pico probe
On the same document check **Appendix A: Using Picoprobe**

Open deploy and run
```
openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program <specific elf file> verify reset exit"
```
