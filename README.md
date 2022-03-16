# ArduinoSecuritySystem
A security system built with Arduino and various electrical components

## Introduction:
This is a very simple security system that is easily broken by cutting the power or just brute-forcing the code. My plan is to install it on my desk drawer but that is not a priority for testing. The keypad will be used for input and the LCD screen will be used for output. The IC will also be used for more access to pins. I also will optionally add a sound alarm to the system which will be elaborated upon below.

## Hardware Components: (18/20) digital, (1/5) analog in
Computing

Elegoo UNO R3 Controller (own) +12 digital pins +5 analog in

74HC595 IC (own) +8 digital pins -3 digital pins

## Inputs
4x4 Matrix Keypad	(purchasing) -8 digital pins

Magnetic Contact Switch (purchasing) -1 analog in 

## Outputs
Solenoid Lock (purchasing) -1 digital pin

LCD Screen (own) -6 output pins

## Other Hardware
Bread DC Barrel jack (purchasing)

Adjustable Power Supply (purchasing)

Breadboard (own)

## Complexities:
Using the 74HC595 IC for access to more pins

Using the 4x4 Keypad for input

Using the LCD screen for output

Using the buzzer for sound (optionally)

Wiring and using the solenoid lock



## Basic Functionality:
The baseline version of the security system is to be able to lock the system, unlock the system, reset the system code, set the system code, and detect when something has been opened while the system is locked using the reed switches. The latter state is known as “intrusion”. If intrusion is detected there will be an alarm sounded.  

 The LCD will display current information about the system in real time. In order to have enough pins for the LCD and the 4x4 keypad, I will have to use the IC’s Shift register. 

When the system is first started it is booted into fresh mode. In fresh mode there is no code and the system cannot be locked. The LCD should display “Fresh”.

User should press “#C” to start code recording. User can enter up to 6 digit code which is finished once “#” is pressed. After input is finished it will show the correct code for 3 seconds.

The system will then go into unlocked mode. In this mode the solenoid is in the “unlocked” position and the alarm is disabled. The LCD will display “unlocked”

User should press “*” to put the system into locked mode. In this mode the solenoid is in the “locked” position and the alarm is enabled. If the magnetic contacts become separated in locked mode the alarm will sound. If the user enters the code the system will enter unlocked mode. Otherwise, the system is stuck in “intruder” mode. The LCD will display “Intruder!!”.

 
 
## Software Libraries Used:

### I used 3 libraries for this project. They were:

Keypad Library

ShiftedLCD library - https://github.com/omersiar/ShiftedLCD

SPI library
