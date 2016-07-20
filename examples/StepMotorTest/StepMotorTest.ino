#include <StepMotor.h>

// Stepper motor test
StepMotor motor[2];

void setup(){
    motor[0] = StepMotor(400, 6, 5, 8, 5, 4);
    motor[1] = StepMotor(400, 11, 12, 9, 10, 13)};
} 

void loop(){
    // Ramp up motor 0
   rampUpAndDown(0, 100);
   rampUpAndDown(1, 100);
}

void rampUpAndDown(int p_m, int p_max_rpm){

    int rpm_increment = 5;
    int steps = p_max_rpm / rpm_increment;
    int pause_time = 2000;
    for(int i = 1; i <= steps; i++){
        motor[p_m].rpm(rpm_increment * (i));
        long start_time = millis();
        while(millis() - start_time < pause_time){}
    }
    for(int i = steps-1; i >= 0 ; i--){
        motor[p_m].rpm(rpm_increment * (i));
        long start_time = millis();
        while(millis() - start_time < pause_time){}
    }
}