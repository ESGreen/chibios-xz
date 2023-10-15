# Burning Man 2020 Badge Instructions

## Main differences from 2017 to 2020 badge:

* Batteries are not interchangeable -- connector polarity is swapped between the two
* New charger circuit -- should have less problems with badges needing a kickstart after a long period of disuse
* No microSD card cage to get tangled in clothing
* Radio Tx/Rx LED status
* Goth color scheme
* "2020" decal on back

## Hardware overview
### Processor
Controller: K22F 64LQFP [Datasheet](https://media.digikey.com/pdf/Data%20Sheets/NXP%20PDFs/K22FPB_Rev.5_Mar3,2014_PB.pdf)
Notes: 
F = Cortext M4 w/DSP & FPU
More text from chip:
N128 VMP10
N128 = 128Kb flash memory
10 = 100MHz
This thing has my grandmothers memory but is fast. 


### Audio
Mic: PH0645LM4H-B i  [Datasheet](https://media.digikey.com/pdf/Data%20Sheets/Knowles%20Acoustics%20PDFs/SPH0645LM4H-B.pdf)

### Mods in this branch.
The main goal of this branch was to add reactive audio. It does this by throwing out as much code as possible adding the ability to analyse the audio signal as described in AudioProcessor.c

## Getting Started

We assume you are building on a Raspberry Pi (so ARM-native) device, and using gcc6. 

1. Check out https://github.com/ESGreen/chibios-xz (git checkout https://github.com/ESGreen/chibios-xz )
2. Check out the bm23_reactive_audio branch (cd chibios-xy; git checkout bm23_reactive_audio)
3. Get the libraries. There is a readme in the src/lib directory that has all the details.
4. Change to the "src" dir
5. Run "make -j3".  If you're cross-compiling it, add " TRGT=arm-none-eabi-" to the command.
6. If you get a complaint about stubs-soft.h, create an empty file of that name in the directory where the error message is pointing to and the error will go away.

The build result will be "build/bm20.elf", an object file that can be loaded using openOCD into the badge.

## Connecting the SWD & Serial via OpenOCD

We'll use the GPIOs on the Raspberry PI to communicate with badge over
the SWD bus to load the firmware.

### Pins Layout on Heart Badge
This is a diagram of the header looking at it from the back side of the board.
The pin numbers are labelled on the headers on the component side of the board (non-OLED side).


| Pin Name | Pin ID | Pin ID | Pin Name |
|----------|--------|--------|----------|
| GRN      |  12 |  11  | TRST (NC) |
| PCS (NC) |  10 |   9  | SWD |            
| SRES     |   8 |   7  | TDO (NC) |
| GND (NC) |   6 |   5  | TDI (NC) |             
| TX       |   4 |   3  | SCK |             
| RX       |   2 |   1  | 3.0V (NC) |

### Pin Connection from Raspberry PI to Heart Badge
| Pi Pin ID  | Pi Pin Name | Badge Pin Name | Badge Pin ID |
|------------|-------------|----------------|--------------|
| PIN 40  | GPIO 21  | SWD   | PIN 9   |
| PIN 38  | GPIO 20  | SCK   | PIN 3   |
| PIN 34  | GND      | GND   | PIN 12  |
| PIN 32  | GPIO 12  | SRES  | PIN 8   |
| PIN  8  | UART0 TX | RX    | PIN 2   |
| PIN 10  | UART0 RX | TX    | PIN 4   |


It's recommended you solder any headers with the pins facing toward the component side (not toward
the OLED side) so that firmware updates are still possible using a pogo pin jig. The pogo pins
come down from the OLED side, so putting the pins facing outward will (a) cause you to be poked
by the pins as you wear the badge and (b) interfere with the pogo pins for reflashing using the
production jig. Use short headers or headers without the typical plastic base. Otherwise, the header pins
will pearce the battery.


### Open OCD
You need to compile OpenOCD from source, and enable "bcm2835gpio".  Install the toolchain.  If you're using Raspbian, it's something like this:

    sudo apt-get install build-essential libtool gdb which
    git clone --recursive git://git.code.sf.net/p/openocd/code openocd
    cd openocd
    ./bootstrap
    ./configure --enable-bcm2835gpio --enable-sysfsgpio --disable-werror
    make
    sudo make install

Then, run OpenOCD:

    cd chibios-bm20/src
    sudo openocd -f bcm-rpi.cfg

## Terminal on the Serial Line
ChibiOS provides a simple terminal on the serial line. There are a number of commands that are accessible. You can write your own to test as you need. This version has removed many of the commands to save memory. But you can run:

fx list
fx next
fx prev

An easy way to connect to the serial port on the raspberry pi is to use minicom. Some setup will be needed to use the serial port (and not as a terminal to the pi) as well as user permissions for the serial port but that is documented everywhere better than here. 

minicom -D /dev/ttyS0


## GDB Loading and Debinggin

If you've compiled your program using a normal toolchain, you can use GDB to load code and debug the software.  From the pi you could run:

    gdb-multiarch -ex "target remote localhost:3333" build/bm20.elf
   
### Loading the ELF File

In gdb, run "load [path-to-build]/build/bm20.elf" to upload the new OS to the board.

To look at OS threads in GDB, add the symbols from the orchard.elf file you built at load address 0:

    (gdb) add-symbol-file [path-to-orchard.elf] 0

You should now be able to look at threads using "info thr", and change threads with "thr [pid]".
