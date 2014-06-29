/* 
   this example from the NESpad Arduino library
   displays the buttons on the joystick as bits
   on the serial port - rahji@rahji.com
   
   Version: 1.3 (11/12/2010) - got rid of shortcut constructor - seems to be broken
   
*/
#include "Arduino.h"
#include "NESpad.h"

// put your own strobe/clock/data pin numbers here -- see the pinout in readme.txt
//NESpad nintendo = NESpad(10,11,12);
NESpad nintendo = NESpad(A3, A2, A1);

byte state = 0;

void setup() {
  Serial.begin(57600);  
}

void loop() {
  
  state = nintendo.buttons();

  // shows the shifted bits from the joystick
  // buttons are high (1) when up 
  // and low (0) when pressed
  Serial.println(state, BIN);

  delay(500);
}
