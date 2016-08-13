# StepMotor
A library for running up to two stepper motors from an Arduino Uno or compatible board connected to 
A4988 motor drivers (available [here](https://www.pololu.com/product/1182) as breakout boards).

## Overview
- This library is currently for continuous motion applications only. There are no fixed distance move functions yet, but they will be added in the next version.
- This library uses the [Timer1](http://playground.arduino.cc/Code/Timer1)library to trigger the step ISRs, so you cannot use timer1 for anything else in your sketch and their associated PWM outputs are disabled (will vary depending on the board).
- This library is written for the Atmega328 chip (Arduino Uno, Arduino Duemilanove, for example). If you use it with other boards, you will probably have to change the port registers used in the ISRs and assignment of the bit masks used to toggle those registers to match your hardware.
- Only two StepMotor objects can be created, since there are only two interrupt timers to which their ISRs can be attached.
Don't try to create more than two. You will be unhappy.

## Caveats
Be careful if using this library with any other code that depends upon interrupts. My initial use of this library was inconjunction with the [Adafruit RGB LCD shield](https://www.adafruit.com/product/714), but that library depends upon the Wire library (I2C), which is interrupt driven. Eventually I was able to find a stable situation in which I could properly update the motor speeds on the LCD, but if I tried to write too many characters or write them at the wrong time, it would make one or both motors cease functioning.

## Documentation

**StepMotor(int p_mot_steps)**

*int p_mot_steps:* The number of physical steps on your motor. Most motors will have 200 (1.8 degree step size), some
will have 400 (0.9 degree) step size.

**void ms(int p_ms)**

*int p_ms:* The new microstep setting the driver should use. Valid values for the A4988 are 1, 2, 4, 8, and 16.

**int ms()**

Returns the current microstep value.

**void rpm(float p_rpm)**

This is the function used for setting the motor's continuous speed. Direction is set by using positive or negative value.
The step rate in steps per second can be checked with the stepsPerSec() function, but cannot be set directly, since the
motor's microstep setting is set automatically based on the RPM.

*float p_rpm:* The new speed of the motor in RPM.

**float rpm()**

Returns the current motor speed in RPM.

**float stepsPerSec()**

Returns the current motor speed in steps per second.

**void flip(bool p_enabled)**

*bool p_enabled:* Whether the motor's direction flags should be flipped.

**bool flip()**

Returns whether the motor's direction flags are being flipped.

**bool updateRequired()**

Returns true if the speed has changed since the last time this function was called. This is useful if the speed is being
displayed on an external device such as an LCD. This is simply a convenience function that avoid the need for a  variable
in some external code to keep track of the last motor speed to compare to the current speed.

**long stepDelay()**

Returns the step period in microseconds.
