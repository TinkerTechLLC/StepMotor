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
    StepMotor(int p_mot_steps, int p_step_pin, int p_dir_pin, int p_ms_pin1, int p_ms_pin2, int p_ms_pin3);

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
    float m_steps_per_sec;
    float m_target_steps_per_sec;
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
    long m_step_delay;
    bool m_update_required;
    bool m_selected;
    

    // Consts
    static const int g_SEC_PER_MIN = 60;
    static const long g_MICROS_PER_SEC = 1e6;

    // Wonky step functions, function pointers, and bitmasks
    static void (*g_ptr_toggleStepPin0)(unsigned char p_on);
    static void (*g_ptr_toggleStepPin1)(unsigned char p_on);
    static void togglePORTD(unsigned char p_on);
    static void togglePORTB(unsigned char p_on);
    static void step0();
    static void step1();
    static unsigned char g_on0;     // Step bitmask for motor 0
    static unsigned char g_on1;     // Step bitmask for motor 1

    // Private functions
    void dir(bool p_fwd);
    void updateStepDelay();
};