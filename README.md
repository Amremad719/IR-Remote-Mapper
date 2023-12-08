# IR Remote Mapper

## Overview

IR Remote Mapper is an Arduino project designed to enable the mapping of IR signals from one remote control to another. In my example I have configured it to work with old Sony Hi-Fi systems, it should work with any device with little modifications.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Hardware Components](#hardware-components)
- [Circuit Diagram](#circuit-diagram)
- [Usage](#usage)

## Prerequisites

Before you begin, make sure you have the following:

- Any Arduino board
- IR sensor and IR blaster
- Breadboard and Jumper wires
- Old remote control
- Target device remote codes and protocol
- Arduino IDE to flash the code

## Circuit Diagram

![Circuit Diagram](https://github.com/Amremad719/IR-Remote-Mapper/blob/master/Diagrams/Circuit.png)

## Installation

1. Connect the IR sensor to the Arduino board.
2. Connect the IR blaster to the Arduino board (Make sure you are connected to a PWM Pin).
3. Open the Arduino IDE on your computer.
4. Download the [IR Remote Mapper.ino](https://github.com/Amremad719/IR-Remote-Mapper/blob/master/src/IR%20Remote%20Mapper.ino) arduino code from this repository.

## Usage

1. Modify the code slightly to print the received signal.
2. Point the remote control towards the IR sensor.
3. Press a button on the old remote control to capture the signal.
4. Open the Arduino Serial Monitor to view the captured signal and save it.
5. Modify the code to map the captured signal to the equivalent target device command.
6. Upload the modified code to the Arduino board.
7. Place the IR blaster in front of the target device and ENJOY!

## Contributing

Feel free to suggest additions and modifications to the project and code by forking.

## Issues

If you encounter any issues or bugs with the code please [start an issue](https://github.com/Amremad719/IR-Remote-Mapper/issues/new).
