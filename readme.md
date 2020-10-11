Logitech G25/27 Shifter to AVR  USB
==
This project is for using a G25/27 shifter connected directly to your computer as a HID device without the need for the main G25/27 unit.

Initial support is alo provided for attaching the G25 (and i assume G27) pedal unit.

In Options.h you can enable/disable options to:
- Send Shifter as analog joystick (as well as button presses)
`#define SHIFTER_JOY 1`

- Process and send clutch, brake, accelerator pedals
`#define USE_PEDALS 1`

## Setup ##

This is the pinout of the DSUB9 coming out of the G27 shifter (female)

PIN   | G27 Purpose                          | G27 Wire Color    | ATmega32u4/Arduino
------|--------------                        |------             |----------------
1     | Clock                                | Purple            | PortB.1 = Arduino 15
2     | Serial Button Data                   | Gray              | PortB.3 = Arduino 14
3     | Shift Register Mode Parallel/Serial  | Yellow            | PortB.6 = Arduino 10
4     | X Axis                               | Orange            | PortF.7 = Arduino 18/A0
5     | Ground LEDs                          | White             | PortB.2 = Arduino 16
6     | Ground Logic                         | Black             | GND
7     | +5V Sense                            | Red               |
8     | Y Axis                               | Green             | PortF.6 = Arduino 19/A1
9     | +5V Supply                           | Red               | VCC



This is the pinout of the DSUB9 coming out of the G25 shifter (female)

PIN   | G25 Purpose                          | G25 Wire Color    | ATmega32u4/Arduino
------|--------------                        |------             |---------------
1     | +5 Sense                             | ??                |
2     | Serial Button Data                   | Gray              | PortB.3 = Arduino 14
3     | Shift Register Mode Parallel/Serial  | Yellow            | PortB.6 = Arduino 10
4     | X Axis                               | Orange            | PortF.7 = Arduino 18/A0
5     | Ground LEDs                          | Red               | PortB.2 = Arduino 16
6     | Ground Logic                         | Black             | GND
7     | Clock                                | Purple            | PortB.1 = Arduino 15
8     | Y Axis                               | Green             | PortF.6 = Arduino 19/A1
9     | +5V Supply                           | Black             | VCC

This is the pinout of the DSUB9 coming out of the G25 pedals (MALE)

PIN   | Pedal Purpose                        | Wire Color    | ATmega32u4/Arduino
------|--------------                        |------         |---------------
1     | +5 Supply                            | ??            | VCC
2     | Accelerator Pedlal Potentiometer     | ??            | PortD.4 = Arduino  4/A6
3     | Brake Pedal Potentiometer            | ??            | PortF.4 = Arduino 21/A3
4     | Clutch Pedal Potentiometer           | ??            | PortF.5 = Arduino 20/A2
5     |                                      | ??            | 
6     | Ground Logic                         | ??            | GND
7     |                                      | ??            | 
8     |                                      | ??            | 
9     |                                      | ??            | 

The first step is to wire the shfiter I/O.

The inputs used by the project are defined in **g27shifter.h**.
As this now uses the SPI bus, you cannot change the GPIO pins that are used for the shift register communication.

After that is connecting the X and Y pins to an ADC port each, these are defined by **STICK_X_ADC** and **STICK_Y_ADC**.

Next is powering the logic circuitry, connect **pin 9** to +5V or VCC and **pin 6** to ground.

Last is connecting **pin 5** to enable the power LED on the shifter.


## Build
Run the make file and flash the hex onto your AVR.

I've tested this with win-avr on a atmega32u4. avr-gcc and Atmels tool chain should work just as well.

## Gear selection is working poorly
You might need to adjust the ADC values for the X and Y axis used used for calculating the selected gear, these are also defined in **g27shifter.h**. Do this by wiring everything up then writing down the voltage on the X and Y pin for each gear in turn with everyting connected to USB.

For example:
Voltage in first might be 1.3 for x.

Now we need to find the ADC value 1.3 and 3.7 corresponds to, since the ADC runs in 10bit mode using VCCREF so measure the voltage from VCC to ground, for example 5V.

The ADC value is then 1.3 / 5 \* 1024. This should be the value for **STICK_X_12**.

Repeat for the other STICK_\* values.

## Differences between G27 and G25
To make this compatible with a G25 shifter move clock from pin 1 to 7 going into the DSUB9.

Since the G27 doesn't have the sequential mode, those buttons will not activate.

## Hardware operation
The X and Y pins are simply potentiometers that tells how the stick is positioned. The buttons work through parallel in serial out shift registers. Pulling the mode pin low then holding it high latches the current input pins so that they can be sequentially read through the serial output. The shift register advances to the next value on the falling edge on the clock pin.

## Schematics
### G25 circuitry ###
![image](http://i.imgur.com/W0HSzhh.png?1)

### G25 and G27 pinouts ###
Showing that 1 and 9 are shorted in a G25

![image](http://i.imgur.com/csH44Uz.jpg?1)

### G25 pedal pinouts ###
![image](https://www.lfs.net/attachment/128450)
