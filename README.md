# PicoFiFun

(**WIP**) Fault injection CTFs with Raspberry Pi Pico, mainly for learning Pico C SDK.

## Setup

[Here](https://www.electronicshub.org/program-raspberry-pi-pico-using-c/) is a good reference for setting up [pico-sdk](https://github.com/raspberrypi/pico-sdk).

The basic steps are the following:

1. Clone the pico-sdk repository and submodules (tinyusb):  
    ```bash
    git clone -b master https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update – -init
    ```

2. Install the toolchain (1.8 GB, takes time and space):
    ```bash
    sudo apt update
    sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
    ```

3. Export the `pico-sdk` path as environmental variable:
    ```bash
    export PICO_SDK_PATH=/home/pi/pico/pico-sdk # changeme
    ```

4. For convenience, create a directory called `projects` in the same directory as `pico-sdk`, where you will copy-paste the `*_src` directory of each writeup.

**Note**: pico-sdk uses CMake to create Make files. For a tidy setup, in each `*_src` directory (e.g. `glitch_src`) create a `build` directory where all such files will be created (`build` folders are already there sometimes).

5. For compilation, enter the specific project's `build` directory and run the following:
    ```bash
    cmake ..
    make -j4 # change number of sim jobs accordingly
    ```
    This will produce .bin, .hex, .elf and .uf2 files.

6. For easy loading of the firmware onto the Pico, I wired up a button to the reset pin (pin 30).  

![reset button](./imgs/reset_button.jpg)
By pressing the "reset" + "BOOTSEL" buttons, then releasing the reset button, the Pico appears as USB storage device (BOOTSEL can be released then).  
The .uf2 file can be copied in the folder, then the Pico restarts and begins running the firmware.

## Target

The target is an ATMEGA328P microcontroller (same as in old Arduino boards), here is the pinout.

![atmega pinout](./imgs/atmega328-pinout.png)

To keep the setting as simple and self-contained as possible I connected the following pins:

- pin 7 (VCC) to Pico pin 40 (VBUS, ~5V)
- pin 8 (GND) to Pico GND (this connection is used for glitching and will be interrupted) 
- a 16 MHz crystal oscillator to pins 9 and 10, with two 22 pF capacitors to ground
- pin 1 (RESET) to pin 7 (VCC, high)

We want to read serial output from the target (pin 3, TXD) so we would need to connect:
- pin 3 of the ATMEGA to Pico's pin 2 (UART0 RX, GP1)

This is done via a voltage divider circuit (the three 1 KOhm resistors in serie from pin 3 to GND) to account for the fact that the ATMEGA operates at 5V and the Pico at 3,3V.

![target stup](./imgs/target_setup.jpg)

The glitch is performed interrupting the connection of the ground line by switching off and on a [2N7000 MOSFET](https://components101.com/mosfets/2n7000-n-channel-mosfet).  
The source and drain pins of the MOSFET are connected to the ATMEGA's and Pico's GND respectively, while the gate pin is connected to Pico's pin 4 (GP2).

![final setup](./imgs/final_setup.jpg)

## Writeups

- [Rhme2016-Fiesta](./rhme2016_fiesta)