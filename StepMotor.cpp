# include "StepMotor.h"

int StepMotor::g_id = 0;
float *StepMotor::g_steps_per_sec = (float *)malloc(g_MAX_MOTORS * sizeof(float));
long *StepMotor::g_step_delay = (long *)malloc(g_MAX_MOTORS * sizeof(long));
const uint8_t StepMotor::g_ON[2] = {B01000000, B00001000};
const int StepMotor::g_STEP[2] = {6, 11};
const int StepMotor::g_DIR[2]  = {7, 12};
const int StepMotor::g_MS1[2]  = {8, 9};
const int StepMotor::g_MS2[2]  = {5, 10};
const int StepMotor::g_MS3[2]  = {4, 13};

StepMotor::StepMotor(){}

StepMotor::StepMotor(int p_mot_steps){
    m_id = g_id++;
    m_mot_steps = p_mot_steps;
    stepPin(g_STEP[m_id]);
    m_dir_pin = g_DIR[m_id];
    m_ms_pin1 = g_MS1[m_id];
    m_ms_pin2 = g_MS2[m_id];
    m_ms_pin3 = g_MS3[m_id];
    m_rpm = 0;

    pinMode(m_step_pin, OUTPUT);
    pinMode(m_dir_pin, OUTPUT);
    pinMode(m_ms_pin1, OUTPUT);
    pinMode(m_ms_pin2, OUTPUT);
    pinMode(m_ms_pin3, OUTPUT);

    if(m_id == 0){
        Timer1.initialize();
    }
    else{
        FrequencyTimer2::disable();
        FrequencyTimer2::setOnOverflow(0);
    }
    // Default to zero speed and full stepping
    g_steps_per_sec[m_id] = 0;
    g_step_delay[m_id] = 0;
    this->ms(16);
}

void StepMotor::stepPin(int p_pin){
    m_step_pin = p_pin;
}
int StepMotor::stepPin(){
    return m_step_pin;
}

void StepMotor::dirPin(int p_pin){
    m_dir_pin = p_pin;
}
int StepMotor::dirPin(){
    return m_dir_pin;
}

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

void StepMotor::rpm(float p_rpm){
    m_rpm = p_rpm;
    const int THRESH_1 = 15;
    const int THRESH_2 = 75;
    const int THRESH_3 = 125;
    const int THRESH_4 = 200;

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
    float steps_per_sec = (float)abs(m_rpm) * (float)m_mot_steps * (float)m_ms / (float)g_SEC_PER_MIN;
    // Update the direction pin
    dir(m_rpm >= 0 ? true : false);
    stepsPerSec(steps_per_sec);
}
float StepMotor::rpm(){
    return m_rpm;
}
void StepMotor::stepsPerSec(float p_steps_per_sec){
    g_steps_per_sec[m_id] = p_steps_per_sec;
    updateStepDelay();
}
float StepMotor::stepsPerSec(){
    return g_steps_per_sec[m_id];
}
long StepMotor::stepDelay(){
    return g_step_delay[m_id];
}
void StepMotor::updateStepDelay(){
    // Don't do anything if neither motor has been initialized
    if(g_id == 0)
        return;
    
    // Stop interrupt if steps per sec is 0. Can't divide by 0 later.
    if(g_steps_per_sec[m_id] == 0){
        if(m_id == 0)
            Timer1.detachInterrupt();
        else{
            FrequencyTimer2::setOnOverflow(NULL);
            FrequencyTimer2::disable();
        }
        return;
    }

    // Determine delay and set new ISR interval
    g_step_delay[m_id] = round((float)g_MICROS_PER_SEC / g_steps_per_sec[m_id]);
    if(m_id == 0){
        Timer1.attachInterrupt(step0, g_step_delay[m_id]);
    }
    else{
        static bool isr_set = false;
        if(!isr_set)
            FrequencyTimer2::setOnOverflow(step1);
        FrequencyTimer2::setPeriod(g_step_delay[m_id]);
        FrequencyTimer2::enable();
    }
    m_update_required = true;
}
void StepMotor::flip(bool p_flip){
    m_flip = p_flip;
    // Force direction setting for new
    dir(m_dir);
}
bool StepMotor::flip(){
    return m_flip;
}
void StepMotor::dir(bool p_fwd){
    m_dir = p_fwd;
    bool set_dir = m_flip ? !m_dir : m_dir;
    digitalWrite(m_dir_pin, set_dir);
}

void StepMotor::select(bool p_selected){
    m_selected = p_selected;
}
bool StepMotor::isSelected(){
    return m_selected;
}

bool StepMotor::updateRequired(){
    bool ret = m_update_required;
    m_update_required = false;
    return ret;
}

void StepMotor::step0(){
    PORTD |= g_ON[0];
    delayMicroseconds(1);
    PORTD &= (g_ON[0] ^ B11111111);
}

void StepMotor::step1(){
    PORTB |= g_ON[1];
    delayMicroseconds(1);
    PORTB &= (g_ON[1] ^ B11111111);
}

void StepMotor::printID(){
    Serial.print("Motor ");
    Serial.print(m_id);
}
