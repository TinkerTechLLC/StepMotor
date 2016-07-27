#ifndef _STEPMOTOR_h
#define _STEPMOTOR_h
#endif

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"    
#else
	#include "WProgram.h"
#endif

#include <TimerOne.h>
#include <FrequencyTimer2.h>

class StepMotor{
    
    public:

    // Constructors
    StepMotor();
    StepMotor(int p_mot_steps);

    void initialize();

    // Public functions
    void ms(int p_ms);
    int ms();

    void rpm(float p_rpm);
    float rpm();

    void stepsPerSec(float p_spd);
    float stepsPerSec();

    void flip(bool p_enabled);
    bool flip();

    void select(bool p_selected);
    bool isSelected();

    void stepPin(int p_pin);
    int stepPin();

    void dirPin(int p_pin);
    int dirPin();

    bool updateRequired();
    long stepDelay();

    private:

    int m_id;
    static int g_id;
    
    //Basic properties
    int m_ms;
    static float *g_steps_per_sec;
    float m_target_steps_per_sec;
    float m_rpm;
    int m_accel;
    bool m_dir;
    bool m_flip;

    // Physical properties
    int m_mot_steps;
    int m_dir_pin;
    int m_step_pin;
    int m_ms_pin1;
    int m_ms_pin2;
    int m_ms_pin3;

    // Derived properties
    static long *g_step_delay;
    bool m_update_required;
    bool m_selected;
    

    // Consts
    static const int g_SEC_PER_MIN = 60;
    static const long g_MICROS_PER_SEC = 1e6;
    static const int g_MAX_MOTORS = 2;
    static const int g_MAX_STEPS_PER_SEC = 4000;
    static const int g_STEP[2];
    static const int g_DIR[2];
    static const int g_MS1[2];
    static const int g_MS2[2];
    static const int g_MS3[2];
    static const uint8_t g_ON[2];   

    // Step functions
    static void step0();
    static void step1();

    // Private functions
    void dir(bool p_fwd);
    void updateStepDelay();
    void printID();
};