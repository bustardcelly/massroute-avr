// http://bildr.org/2011/08/74hc595-breakout-arduino/

#include <Shifter.h>

int dataPin = 11;
int latchPin = 12;
int clockPin = 8;
int n;

Shifter shifter(11, 12, 8, 2); 

void setup(){

}

void loop(){
 for(n = 0; n < 14; n++) {
   shifter.clear();
   shifter.setPin(n, HIGH);
   shifter.write();
   if(n == 5) {
     n = 7;
   }
   delay(500);
 }
}
