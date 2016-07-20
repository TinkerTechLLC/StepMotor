# include "StepMotor.h"

int StepMotor::g_id = 0;
unsigned char StepMotor::g_on0 = 1;     // Step bitmask for motor 0
unsigned char StepMotor::g_on1 = 1; 
void (*StepMotor::g_ptr_toggleStepPin0)(unsigned char) = NULL;
void (*StepMotor::g_ptr_toggleStepPin1)(unsigned char) = NULL;

StepMotor::StepMotor(){}

StepMotor::StepMotor(int p_mot_steps, int p_step_pin, int p_dir_pin, int p_ms_pin1, int p_ms_pin2, int p_ms_pin3){
    m_id = g_id++;
    m_mot_steps = p_mot_steps;
    m_step_pin = p_step_pin;
    m_dir_pin = p_dir_pin;
    m_ms_pin1 = p_ms_pin1;
    m_ms_pin2 = p_ms_pin2;
    m_ms_pin3 = p_ms_pin3;

    pinMode(m_step_pin, OUTPUT);
    pinMode(m_dir_pin, OUTPUT);
    pinMode(m_ms_pin1, OUTPUT);
    pinMode(m_ms_pin2, OUTPUT);
    pinMode(m_ms_pin3, OUTPUT);

    if(m_id == 0){
        Timer1.initialize();
    }
    else{
        FrequencyTimer2::disable();                 // This disables toggling of pin 11 at every interrupt
        FrequencyTimer2::setOnOverflow(0);          // Initially set the interrupt function to null
    }

    // Default to zero speed and full stepping
    m_steps_per_sec = 0;
    this->ms(1);
}

void StepMotor::stepPin(int p_pin){
    m_step_pin = p_pin;
    // Set the bit masks for pin toggling
    if(m_step_pin >= 0 && m_step_pin <= 7){
        if(m_id == 0){
            g_ptr_toggleStepPin0 = &StepMotor::togglePORTD;
            g_on0 = 1 << m_step_pin;
        }
        else{
            g_ptr_toggleStepPin1 = &StepMotor::togglePORTD;
            g_on1 = 1 << m_step_pin;
        }
    }
    else if(m_step_pin >= 8 && m_step_pin <= 13){
        if(m_id == 0){
            g_ptr_toggleStepPin0 = &StepMotor::togglePORTB;
            g_on0 = 1 << (m_step_pin - 8);
        }
        else{
            g_ptr_toggleStepPin1 = &StepMotor::togglePORTB;
            g_on1 = 1 << (m_step_pin - 8);
        }
    }
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
    // Don't update anything if the setting didn't change
    if(p_ms == m_ms)
        return;

    m_ms = p_ms;
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
        digitalWrite(m_ms_pin1, LOW);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, HIGH);
        break;

        case 16:
        digitalWrite(m_ms_pin1, HIGH);
        digitalWrite(m_ms_pin2, HIGH);
        digitalWrite(m_ms_pin3, HIGH);
        break;
    }
    updateStepDelay();
}
int StepMotor::ms(){
    return m_ms;
}

void StepMotor::rpm(float p_rpm){
    float steps_per_sec = p_rpm / (g_SEC_PER_MIN * m_mot_steps * m_ms);
    stepsPerSec(steps_per_sec);
}
float StepMotor::rpm(){
    float RPM = (m_mot_steps * m_ms / m_steps_per_sec) * g_SEC_PER_MIN;
    return RPM;
}
void StepMotor::stepsPerSec(float p_steps_per_sec){
    m_steps_per_sec = p_steps_per_sec;
    // Update the direction pin
    dir(m_steps_per_sec >= 0 ? true : false);
    updateStepDelay();
}
float StepMotor::stepsPerSec(){
    return m_steps_per_sec;
}
long StepMotor::stepDelay(){
    return m_step_delay;
}
void StepMotor::updateStepDelay(){
    if(m_steps_per_sec == 0){
        if(m_id == 0){
            Timer1.detachInterrupt(); 
        }
        else{
            FrequencyTimer2::setOnOverflow(0); 
        }
    }
    else{
        m_step_delay = round((float)g_MICROS_PER_SEC / m_steps_per_sec);
        if(m_id == 0){
            Timer1.attachInterrupt(step0, m_step_delay >= 0 ? m_step_delay : -m_step_delay);
        }
        else{
            FrequencyTimer2::setOnOverflow(step1);
            FrequencyTimer2::setPeriod(m_step_delay >= 0 ? m_step_delay : -m_step_delay);
        }
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
    digitalWrite(m_dir, set_dir);
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
    g_ptr_toggleStepPin0(g_on0);
}

void StepMotor::step1(){
    g_ptr_toggleStepPin1(g_on1);
}

void StepMotor::togglePORTD(unsigned char p_on){
    PORTD |= p_on;
    delayMicroseconds(1);
    PORTD &= (!p_on);
}

void StepMotor::togglePORTB(unsigned char p_on){
    PORTB |= p_on;
    delayMicroseconds(1);
    PORTB &= (!p_on);
}
