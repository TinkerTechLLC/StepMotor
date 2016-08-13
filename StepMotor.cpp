# include "StepMotor.h"

// Initialize static variables and allocate memory for static arrays
int StepMotor::g_id = 0;
float *StepMotor::g_steps_per_sec = (float *)malloc(g_MAX_MOTORS * sizeof(float));
const uint8_t StepMotor::g_ON[2] = {B01000000, B00001000};      // These values are specific to the ATmega328P PORTD and PORTB registers
// These pin values correspond to the TinkerTech Stepper Motor Shield
const int StepMotor::g_STEP[2] = {6, 11};
const int StepMotor::g_DIR[2]  = {7, 12};
const int StepMotor::g_MS1[2]  = {8, 9};
const int StepMotor::g_MS2[2]  = {5, 10};
const int StepMotor::g_MS3[2]  = {4, 13};
volatile long *StepMotor::g_cycle_err = (long *)malloc(g_MAX_MOTORS * sizeof(long));
volatile long *StepMotor::g_total_err = (long *)malloc(g_MAX_MOTORS * sizeof(long));
volatile long *StepMotor::g_off_cycles = (long *)malloc(g_MAX_MOTORS * sizeof(long));
volatile long *StepMotor::g_cur_off_cycles = (long *)malloc(g_MAX_MOTORS * sizeof(long));
volatile bool *StepMotor::g_running = (bool *)malloc(g_MAX_MOTORS * sizeof(bool));

/*
 *  Default constructor for object declaration without instantiation.
 */
StepMotor::StepMotor(){}

/*
 *  Constructor with object initialization.
 *
 *  @param p_mot_steps
 *          The number of physical steps per roation for this motor. Typical values
 *          are 200 and 400 steps.
 */
StepMotor::StepMotor(int p_mot_steps){
    // Set the motor's id and pin values
    m_id = g_id++;
    m_mot_steps = p_mot_steps;
    stepPin(g_STEP[m_id]);
    m_dir_pin = g_DIR[m_id];
    m_ms_pin1 = g_MS1[m_id];
    m_ms_pin2 = g_MS2[m_id];
    m_ms_pin3 = g_MS3[m_id];
    m_rpm = 0;

    // Make sure all motor control pins are in OUTPUT mode
    pinMode(m_step_pin, OUTPUT);
    pinMode(m_dir_pin, OUTPUT);
    pinMode(m_ms_pin1, OUTPUT);
    pinMode(m_ms_pin2, OUTPUT);
    pinMode(m_ms_pin3, OUTPUT);

    // If this is the first motor to be initialized, attach the
    // ISR to the Timer1 interrupt.
    if(m_id == 0){
        Timer1.initialize();
        Timer1.attachInterrupt(_ISR, g_ISR_INTERVAL);
    }

    // Default to zero speed and full stepping
    g_steps_per_sec[m_id] = 0;
    // Default to 16th stepping
    this->ms(16);
}

/*
 *  Private function that sets the motor's step pin.
 *
 *  @param p_pin
 *          The Arduino pin that will be used for stepping.
 */
void StepMotor::stepPin(int p_pin){
    m_step_pin = p_pin;
}

/*
 *  Returns the motor's step pin number
 *
 *  @return
 *          The Arduino pin that will be used for stepping.
 */
int StepMotor::stepPin(){
    return m_step_pin;
}

/*
 *  Private function that sets the motor's direction pin.
 *
 *  @param p_pin
 *          The Arduino pin that will be used for direction.
 */
void StepMotor::dirPin(int p_pin){
    m_dir_pin = p_pin;
}

/*
 *  Returns the motor's direction pin number
 *
 *  @return
 *          The Arduino pin that will be used for direction.
 */
int StepMotor::dirPin(){
    return m_dir_pin;
}

/*
 *  Sets the motor's micro step setting and updates the steps / sec speed rate._ISR
 *
 *  @param p_ms
 *          The motor's microstep setting. Value values are 1, 2, 4, 8, and 16.
 */
void StepMotor::ms(int p_ms){

    int new_steps_per_sec = (int)((float)g_steps_per_sec[m_id] * (float)p_ms / (float)m_ms);

    /*
        When "down shifting" (i.e. increasing microstepping), the new steps per second
        should be set before changing the microstep pins because the new rate will need
        to be faster, so changing the pins while the ISR rate is too low can cause the
        motor to stall. The reverse is true when "up shifting", so in that case the new
        ISR rate should be changed after toggling the microstep pins.
    */
    bool down_shift = p_ms > m_ms ? true : false;
    m_ms = p_ms;
    if(down_shift)
        stepsPerSec(new_steps_per_sec);
    switch(m_ms){
        case 1:
        digitalWrite(m_ms_pin1, LOW);
        digitalWrite(m_ms_pin2, LOW);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 2:
        digitalWrite(m_ms_pin1, HIGH);
        digitalWrite(m_ms_pin2, LOW);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 4:
        digitalWrite(m_ms_pin1, LOW);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 8:
        digitalWrite(m_ms_pin1, HIGH);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, LOW);
        break;

        case 16:
        digitalWrite(m_ms_pin1, HIGH);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, HIGH);
        break;
    } 
    if(!down_shift)
        stepsPerSec(new_steps_per_sec);
}
int StepMotor::ms(){
    return m_ms;
}

/*
 *  Sets the motor's new speed in RPM and auto-sets the motor'select
 *  micro step rate in order to optimize smoothness of motion. Then
 *  updates the motor's steps / second speed rate.
 *
 *  @param p_rpm
 *          Speed in RPM as a float value
 */
void StepMotor::rpm(float p_rpm){
    // Save the newly set RPM
    m_rpm = p_rpm;

    // Thresholds at which the microstep setting should change
    const int THRESH_1 = 15;
    const int THRESH_2 = 75;
    const int THRESH_3 = 125;
    const int THRESH_4 = 200;

    // Set the microstep rate based upon the newly set RPM
    if(m_rpm <= THRESH_1 && m_rpm >= -THRESH_1){
        ms(16);
    }
    else if(m_rpm > THRESH_1 && m_rpm <= THRESH_2 ||
        m_rpm < -THRESH_1 && m_rpm >= -THRESH_2){
        ms(8);
    }
    else if(m_rpm > THRESH_2 && m_rpm <= THRESH_3 ||
        m_rpm < -THRESH_2 && m_rpm >= -THRESH_3){
        ms(4);
    }

    else if(m_rpm > THRESH_3 && m_rpm <= THRESH_4 ||
        m_rpm < -THRESH_3 && m_rpm >= -THRESH_4){
        ms(2);
    }

    else if(m_rpm > THRESH_4 || m_rpm < -THRESH_4){
        ms(1);
    }

    // Determin the new steps / second speed rate
    float steps_per_sec = (float)abs(m_rpm) * (float)m_mot_steps * (float)m_ms / (float)g_SEC_PER_MIN;

    // Update the direction pin
    dir(m_rpm >= 0 ? true : false);
    
    // Update the steps / sec speed rate
    stepsPerSec(steps_per_sec);
}

/*
 *  Returns the motor's speed in RPM
 *  @return
 *          A float value representing the motor's speed in RPM
 */
float StepMotor::rpm(){
    return m_rpm;
}

/*
 * Private function for setting the motor's current speed
 * in steps per second.
 *
 *  @param p_steps_per_sec
 *              Motor's speed in steps / second
 */
void StepMotor::stepsPerSec(float p_steps_per_sec){
    g_steps_per_sec[m_id] = p_steps_per_sec;
    updateStepDelay();
}

/*
 *  Returns the motor's current speed in steps per second
 *  @return
 *          float value of steps / second
 */
float StepMotor::stepsPerSec(){
    return g_steps_per_sec[m_id];
}

/*
 *  Based upon the currently set RPM value, determines how long
 *  the motor should wait between steps and how much error should
 *  accumulate with each step.
 */
void StepMotor::updateStepDelay(){
    // Set g_running false to prevent divide by 0 errors if speed is 0
    if(g_steps_per_sec[m_id] == 0){
        g_running[m_id] = false;
        return;
    }
    else{
        g_running[m_id] = true;
    }

    // Determine delay and set new ISR interval
    float step_delay = floor((float)g_MICROS_PER_SEC / g_steps_per_sec[m_id]);
    float temp_off_cycles = step_delay / g_ISR_INTERVAL;
    g_off_cycles[m_id] = (long) temp_off_cycles;  // Truncates float value, doesn't round
    g_cycle_err[m_id] = (temp_off_cycles - (long) temp_off_cycles) * g_FLOAT_CONVERT;
    g_total_err[m_id] = 0;
}

/*
 *  Sets whether the motor fwd/bkwd directions should be flipped.
 *
 *  @param p_flip
 *          A bool indicating whether the directions should be flipped.
 */
void StepMotor::flip(bool p_flip){
    m_flip = p_flip;
    // Set direction so flip change takes effect
    dir(m_dir);
}

/*
 *  Checks the motor's direction flip state
 *
 *  @return
 *          A boolean indicating whether the motor fwd/bkwd directions are flipped.
 */
bool StepMotor::flip(){
    return m_flip;
}

/*
 * Sets the motor's direction. True for forward,
 * false for backward, unless the "m_flip" flag
 * is true, in which case the directions are reversed.
 */
void StepMotor::dir(bool p_fwd){
    m_dir = p_fwd;
    bool set_dir = m_flip ? !m_dir : m_dir;
    digitalWrite(m_dir_pin, set_dir);
}

/*
 *  Prints the motor's ID number to serial.
 */
void StepMotor::printID(){
    if(Serial){
        Serial.print("Motor ");
        Serial.println(m_id);
    }
}

/*
 *  This static function determines whether the selected motor should
 *  take a step, wait longer, or skip its current step because its errors
 *  counter has overflowed._ISR
 *
 *  @param p_which
 *          The id number of the motor that should be checked.
 *  @return
 *          A boolean indicating whether the motor should step now.
 */
boolean StepMotor::checkStep(int p_which){
    if(g_total_err[p_which] > 2 * g_FLOAT_CONVERT){
        g_total_err[p_which] = 0;
    }
    if(g_total_err[p_which] > g_FLOAT_CONVERT){
        g_total_err[p_which] -= g_FLOAT_CONVERT;
        return false;
    }

    g_cur_off_cycles[p_which]++;

    // If enough low cycles have passed
    if(g_cur_off_cycles[p_which] >= g_off_cycles[p_which]){
        // Reset off cycle counter
        g_cur_off_cycles[p_which] = 0;
        g_total_err[p_which] += g_cycle_err[p_which];
        // Serial.println("CS: step");
        return true;
    }
    // ...else not enough low cycles have passed
    else{
        // Serial.println(g_off_cycles[p_which]);
        // Serial.println("CS: wait");
        return false;
    }
}

/*
 * The interrupt service routine, which is responsible for
 * checking whether a motor needs to step and toggling the 
 * appropriate step pin if true.
 */
void StepMotor::_ISR(){
    // Check both motors and step at the same time.
    for(int i = 0; i < g_MAX_MOTORS; i++){
        // If it's time to step, set the appropriate motor step pin high
        if(g_running[i] && checkStep(i)){
            if(i == 0){
                PORTD |= g_ON[0];
            }
            else{
                PORTB |= g_ON[1];
            }
        }
    }

    /*  
     *  Wait for 1uS, but use assembly NOP command instead
     *  of delayMicroseconds() for higher accuracy and for
     *  non-blocking purposes.
     */
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"); 
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"); 
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"); 
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Set both motor step pins low
    PORTD &= (g_ON[0] ^ B11111111);
    PORTB &= (g_ON[1] ^ B11111111);
}