/*
 *  The TinkerTech stepper motor shield uses a lot of
 *  pins. Make sure you've consulted the shield's documentation
 *  and know which ones are going to be commendeered before
 *  trying to control any of the digital pins.
 *
 *  https://github.com/TinkerTechLLC/StepMotor/blob/master/README.md
 */

#include <TimerOne.h>
#include <StepMotor.h>

// Stepper motor test
StepMotor motor[2];
int low_min =0;

void setup(){
    Serial.begin(9600);

    // Adjust this value to match the number of physical steps on your motor
    int mot_steps = 200;
    motor[0] = StepMotor(mot_steps);
    motor[1] = StepMotor(mot_steps);

    motor[0].ms(4);
    motor[1].ms(4); 
    motor[0].rpm(0); 
    motor[1].rpm(0);
} 

void wait(int wait_time){
  long start_time = millis();
  while(millis() - start_time < wait_time){}
}

void loop(){
  
   int max_ramp = 100;
 
   rampUp(0, max_ramp);
   wait(5000);
   rampDown(0, max_ramp);
   wait(2000);
   
   rampUp(1, max_ramp);
   wait(5000);
   rampDown(1, max_ramp);
   wait(2000);

   rampUpAndDown(0, max_ramp);
}

void rampUp(int p_m, int p_max_rpm){
    float rpm_increment = 0.5;
    int steps = (int)((float)p_max_rpm / rpm_increment);
    int pause_time = 45;
    
    Serial.println("Ramping up");    
    // Ramp up
    for(int i = low_min; i <= steps; i++){
        float new_spd = rpm_increment * i;
        motor[p_m].rpm(new_spd);
        long start_time = millis();
        while(millis() - start_time < pause_time){}
    }
    
    Serial.println("Now at max speed");
}

void rampDown(int p_m, int p_max_rpm){
      float rpm_increment = 1;
    int steps = (int)((float)p_max_rpm / rpm_increment);
    int pause_time = 85;
    
    Serial.println("Ramping down");    
    for(int i = steps; i >= low_min; i--){
        float new_spd = rpm_increment * i;
        //Serial.print("\nAssigning new speed: ");
        //Serial.println(new_spd); 
        motor[p_m].rpm(new_spd);
        long start_time = millis();
        while(millis() - start_time < pause_time){}
    }
}

void rampUpAndDown(int p_m, int p_max_rpm){
    rampUp(p_m, p_max_rpm);
    wait(5000);
    rampDown(p_m, p_max_rpm);
}
