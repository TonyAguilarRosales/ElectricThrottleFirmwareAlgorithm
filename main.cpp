#include "mbed.h"
#include <cmath>


//maps input voltage to a percentage based on min and max voltages
float linearPedalMapping(float voltage, float Vmin, float Vmax){
   float range = Vmax - Vmin;
   float percent = (voltage - Vmin) / range * 100.0;
   
   if(percent < 0.0) {
      percent = 0.0;
   }
   if(percent > 100.0) {
      percent = 100.0;
   }
   return percent;
}

AnalogIn APPS0{PA_0};
AnalogIn APPS1{PA_4}; 
AnalogIn B_APPS0{PA_5}; //Brake switch input
DigitalIn START_BUTTON{PA_6};
DigitalIn TS_ACTIVE{PA_7};
DigitalOut buzzer{PB_0};  //substitute for buzzer should turn on an LED for 1 second

bool rtd = false;       // Ready to drive state
bool rtd_prev = false;  

// ----------SENSOR MIN/MAX VOLTAGES change each according to the sensor datasheet)
float APPS0_MIN_VOLT = 0.25;
float APPS0_MAX_VOLT = 2.25;
float APPS1_MIN_VOLT = 0.3;
float APPS1_MAX_VOLT = 2.7;
float B_APPS0_MIN_VOLT = 0.25;
float B_APPS0_MAX_VOLT = 2.25;

float throttle_percent = 0.0;
bool implausibility = true;


int main() {
   while(true){
      //if not ready to drive read brake + start button + TS_active
      if(!rtd){
         float brake_voltage = B_APPS0.read() * 3.3; //reads voltage from PA_5
         float brake_percent = linearPedalMapping(brake_voltage, B_APPS0_MIN_VOLT, B_APPS0_MAX_VOLT);
         //returns a percentage based on min and max voltages for brake switch
         //if all 3 conditions are met we are ready to drive
         if((brake_percent >= 80.0) && START_BUTTON.read() && TS_ACTIVE.read()){ 
            rtd = true;
            buzzer = 1; //turn on buzzer
            ThisThread::sleep_for(1500ms); //buzzer on for 1.5 seconds
            buzzer = 0; //turn off buzzer
            rtd_prev = rtd;
         }
      }

      if(rtd){
         //Min and Max voltages are subject to change!!(CHECK DATASHEET)***********************

         //read both voltages and convert to percentage
         float apps0_voltage = APPS0.read() * 3.3; //reads voltage from PA_0
         float apps1_voltage = APPS1.read() * 3.3; //reads voltage from PA_4

         float apps0_percent = linearPedalMapping(apps0_voltage, APPS0_MIN_VOLT, APPS0_MAX_VOLT);
         //returns a percentage based on min and max voltages for apps0
         float apps1_percent = linearPedalMapping(apps1_voltage, APPS1_MIN_VOLT, APPS1_MAX_VOLT);
         //returns a percentage based on min and max voltages for apps1

         //check for voltage implausibility
         if(apps0_voltage < APPS0_MIN_VOLT || apps0_voltage > APPS0_MAX_VOLT ||
            apps1_voltage < APPS1_MIN_VOLT || apps1_voltage > APPS1_MAX_VOLT || 
            fabsf(apps0_percent - apps1_percent) > 10.0){
            implausibility = true;
            throttle_percent = 0.0; //set throttle to 0 if implausibility detected
            printf("0\r\n"); //Print 0 while wheels should NOT spin
         }
         else{
            implausibility = false;
            throttle_percent = (apps0_percent + apps1_percent) / 2.0;
            printf("%.2f\r\n", throttle_percent); //Print average throttle percent

         }
      }
         ThisThread::sleep_for(50ms); //delay between loops
   }
}
