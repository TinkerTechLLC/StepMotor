# include "StepMotor.h"

int StepMotor::g_id = 0;
void (*StepMotor::g_ptr_toggleStepPin0)(uint8_t) = NULL;
void (*StepMotor::g_ptr_toggleStepPin1)(uint8_t) = NULL;
float *StepMotor::g_steps_per_sec = (float *)malloc(g_MAX_MOTORS * sizeof(float));
long *StepMotor::g_step_delay = (long *)malloc(g_MAX_MOTORS * sizeof(long));
long *StepMotor::g_last_step_time = (long *)malloc(g_MAX_MOTORS * sizeof(long));
uint8_t *StepMotor::g_on = (uint8_t *)malloc(g_MAX_MOTORS * sizeof(uint8_t));

StepMotor::StepMotor(){}

StepMotor::StepMotor(int p_mot_steps, int p_step_pin, int p_dir_pin, int p_ms_pin1, int p_ms_pin2, int p_ms_pin3){
    m_id = g_id++;
    m_mot_steps = p_mot_steps;
    stepPin(p_step_pin);
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
        //Timer1.attachInterrupt(checkStep, 30);//, 1e6 / g_MAX_STEPS_PER_SEC);
    }
    // Default to zero speed and full stepping
    g_steps_per_sec[m_id] = 0;
    g_last_step_time[m_id] = micros();
    this->ms(16);
}

void StepMotor::stepPin(int p_pin){
    m_step_pin = p_pin;
    // Set the bit masks for pin toggling
    if(m_step_pin >= 0 && m_step_pin <= 7){
        if(m_id == 0){
            g_ptr_toggleStepPin0 = &StepMotor::togglePORTD;
            Serial.println("MOTOR 0, PORT D");
        }
        else{
            g_ptr_toggleStepPin1 = &StepMotor::togglePORTD;
            Serial.println("MOTOR 1, PORT D");
        }
        g_on[m_id] = 1 << m_step_pin;
    }
    else if(m_step_pin >= 8 && m_step_pin <= 13){
        if(m_id == 0){
            g_ptr_toggleStepPin0 = &StepMotor::togglePORTB;
            Serial.println("MOTOR 0, PORT B");
        }
        else{
            g_ptr_toggleStepPin1 = &StepMotor::togglePORTB;
            Serial.println("MOTOR 1, PORT B");
        }
        g_on[m_id] = 1 << (m_step_pin - 8);
    }
    printID();
    Serial.print(" on mask: ");
    Serial.println(g_on[m_id], BIN);
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
    if(m_ms == p_ms)
        return;
    printID();
    Serial.print(" Setting ms: ");
    Serial.print(" new ms: ");
    Serial.println(p_ms);
    int new_steps_per_sec = (int)((float)g_steps_per_sec[m_id] * (float)p_ms / (float)m_ms);
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
    stepsPerSec(new_steps_per_sec);
}
int StepMotor::ms(){
    return m_ms;
}

void StepMotor::rpm(float p_rpm){
    // printID();
    // Serial.print(" RPM: ");
    // Serial.print(p_rpm);
    // Serial.print(" mot steps: ");
    // Serial.print(m_mot_steps);
    // Serial.print(" sec per min: ");
    // Serial.print(g_SEC_PER_MIN);
    // Serial.print(" ms: ");
    // Serial.print(m_ms);
    if(p_rpm <= 20){
        ms(16);
    }
    else if(p_rpm > 20 && p_rpm <= 100){
        ms(8);
    }
    else if(p_rpm > 100 && p_rpm <= 150){
        ms(4);
    }

    else if(p_rpm > 150 && p_rpm <= 200){
        ms(2);
    }

    else if(p_rpm > 200){
        ms(1);
    }
    float steps_per_sec = (float)p_rpm * (float)m_mot_steps * (float)m_ms / (float)g_SEC_PER_MIN;
    Serial.print(" steps per sec: ");
    Serial.println(steps_per_sec);
    stepsPerSec(steps_per_sec);
}
float StepMotor::rpm(){
    float RPM = (m_mot_steps * m_ms / g_steps_per_sec[m_id]) * g_SEC_PER_MIN;
    return RPM;
}
void StepMotor::stepsPerSec(float p_steps_per_sec){
    g_steps_per_sec[m_id] = p_steps_per_sec;
    // Update the direction pin
    dir(g_steps_per_sec[m_id] >= 0 ? true : false);
    updateStepDelay();
}
float StepMotor::stepsPerSec(){
    return g_steps_per_sec[m_id];
}
long StepMotor::stepDelay(){
    return g_step_delay[m_id];
}
void StepMotor::updateStepDelay(){
    if(g_id == 0)
        return;
    g_step_delay[m_id] = round((float)g_MICROS_PER_SEC / g_steps_per_sec[m_id]);
    Timer1.attachInterrupt(step1, g_step_delay[m_id]);//, 1e6 / g_MAX_STEPS_PER_SEC);
    printID();
    Serial.print(" step delay: ");
    Serial.println(g_step_delay[m_id]);
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
    // Serial.print("Setting dir: ");
    // Serial.println(set_dir);
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
    g_ptr_toggleStepPin0(g_on[0]);
}

void StepMotor::step1(){
    g_ptr_toggleStepPin1(g_on[1]);
}

void StepMotor::togglePORTD(uint8_t p_on){
    PORTD |= p_on;
    delayMicroseconds(1);
    PORTD &= (p_on ^ B11111111);
}

void StepMotor::togglePORTB(uint8_t p_on){
    PORTB |= p_on;
    delayMicroseconds(1);
    PORTB &= (p_on ^ B11111111);
}

void StepMotor::printID(){
    Serial.print("Motor ");
    Serial.print(m_id);
}
void StepMotor::checkStep(){
    for(int i = 0; i < g_id ; i++){
        if(g_steps_per_sec[i] == 0 || micros() - g_last_step_time[i] < g_step_delay[i]){
            // Serial.println("NOT TIME YET!!!");
            continue;
        }
        else{
            g_last_step_time[i] = micros();
            if(i == 0){
                step0();
            }
            else if(i == 1){
                // Serial.println("STEPPING 1");
                togglePORTB(g_on[1]);
            }
        }
    }
}
