#ifndef _STEPMOTOR_h
#define _STEPMOTOR_h
#endif

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"    
#else
	#include "WProgram.h"
#endif

// TimerOne must also be included in whatever
// code uses this firmware
#include <TimerOne.h>

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

    float stepsPerSec();

    void flip(bool p_enabled);
    bool flip();

    void select(bool p_selected);
    bool isSelected();

    int stepPin();
    int dirPin();

    bool updateRequired();
    long stepDelay();

    private:

    int m_id;
    static int g_id;
    
    //Basic properties
    int m_ms;
    static float *g_steps_per_sec;
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

    // ISR interval in microseconds
    // Motors run best with an interval around 50us
    static const int g_ISR_INTERVAL = 50;

    static const int g_FLOAT_CONVERT = 1000;
    static volatile long *g_cycle_err;    
    static volatile long *g_total_err;
    static volatile long *g_off_cycles;
    static volatile long *g_cur_off_cycles;
    static volatile bool *g_running;


    // Step functions
    void stepsPerSec(float p_spd);
    static bool checkStep(int p_which);
    static void _ISR();

    // Pin setup functions
    /*
     * These have to be private becuase the ISR
     * is currently setup to toggle fixed pins on 
     * the port registers, so code outside the library
     * shouldn't touch these values
     */
    void stepPin(int p_pin);
    void dirPin(int p_pin);

    // Private functions
    void dir(bool p_fwd);
    void updateStepDelay();
    void printID();
};