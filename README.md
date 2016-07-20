# StepMotor
A library for running up to two stepper motors from an Arduino Uno or compatible board connected to 
A4988 motor drivers (available [here](https://www.pololu.com/product/1182) as breakout boards).

## Overview
- This library uses [Timer1](http://playground.arduino.cc/Code/Timer1) and [FrequencyTimer2](http://playground.arduino.cc/Code/FrequencyTimer2) 
libraries to trigger the step ISRs, so you cannot use timer1 or timer2 for anything else in your sketch and their associated
PWM outputs are disabled (will vary depending on the board).
- This library is written for the Atmega328 chip (Arduino Uno, Arduino Duemilanove, for example). If you use it with other boards,
you will probably have to change the port registers used in the ISRs and assignment of the bit masks used to toggle those 
registers to match your hardware.
- Only two StepMotor objects can be created, since there are only two interrupt timers to which their ISRs can be attached.
Don't try to create more than two. You will be unhappy.

## Documentation

**StepMotor(int p_mot_steps, int p_step_pin, int p_dir_pin, int p_ms_pin1, int p_ms_pin2, int p_ms_pin3)**

*int p_mot_steps:* The number of physical steps on your motor. Most motors will have 200 (1.8 degree step size), some
will have 400 (0.9 degree) step size.

*int p_step_pin:* The pin connected to the motor driver's step input pin.

*int p_dir_pin:* The pin connected to the motor driver's direction input pin.

*int p_ms_pin1:* The pin connected to the motor driver's ms1 input pin.

*int p_ms_pin2:* The pin connected to the motor driver's ms2 input pin.

*int p_ms_pin2:* The pin connected to the motor driver's ms3 input pin.

**void ms(int p_ms)**

*int p_ms:* The new microstep setting the driver should use. Valid values for the A4988 are 1, 2, 4, 8, and 16.

**int ms()**

Returns the current microstep value.

**void rpm(float p_rpm)**

*float p_rpm:* The new speed of the motor in RPM.

**float rpm()**

Returns the current motor speed in RPM.

**void stepsPerSec(float p_spd)**

*float p_spd:* The new speed of the motor in steps per second.

**float stepsPerSec()**

Returns the current motor speed in steps per second.

**void flip(bool p_enabled)**

*bool p_enabled:* Whether the motor's direction flags should be flipped.

**bool flip()**

Returns whether the motor's direction flags are being flipped.

**void select(bool p_selected)**

*bool p_selected:* Sets a "selected" flag for the motor object. This is not used within the object and is merely an indicator
that can be used by external code that may want to indicated an active motor.

**bool isSelected()**

Returns whether the motor's "selected" flag is true.

**bool updateRequired()**

Returns true if the speed has changed since the last time this function was called. This is useful if the speed is being
displayed on an external device such as an LCD. This is simply a convenience function that avoid the need for a  variable
in some external code to keep track of the last motor speed to compare to the current speed.

**long stepDelay()**

Returns the step period in microseconds.
