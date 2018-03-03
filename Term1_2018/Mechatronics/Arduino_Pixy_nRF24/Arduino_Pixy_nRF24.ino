/*
* Pixy GPS
* The following code broadcasts the X/Y coordinates of beacons recognized by the Pixy Camera
* Receivers may use this code as an example for receiving the GPS signal (change "radioNumber" to 0)
* Updated: Mar 3, 2018 by Mo Woods
*/

/****************** Libraries ***************************/
//   - Both devices use SPI
#include <SPI.h>

//   - Pixy Camera
// TUTORIAL: http://cmucam.org/projects/cmucam5/wiki/Hooking_up_Pixy_to_a_Microcontroller_(like_an_Arduino)
#include <PixySPI_SS.h>
//#include <Pixy.h>

//   - nRF24 radio modules
// TUTORIAL: http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
#include "RF24.h"

/****************** Config ***************************/
static unsigned long ticks = millis();
bool radioNumber = 1; // Set this radio as radio number 0 (RX) or 1 (TX)

RF24 radio(7,8); // Set up nRF24L01 radio on SPI bus plus pins 7 & 8
PixySPI_SS pixy(10); // This is the main Pixy object 

byte addresses[][6] = {"1Node","2Node"};

void setup() {
  Serial.begin(115200);
  
  radio.begin();
  pixy.init();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber==1){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
}

void loop() {
  if(radioNumber == 1) TX();
  else RX();
}

void TX(void){
  int j;
  uint16_t blocks;
  char buf[32];

  // Prevent the radio from listening for data
  radio.stopListening();
  
  /**** Get Data *****/
  blocks = pixy.getBlocks();
  if (millis()-ticks>500){ // Slow down report rate to avoid bogging down system
    if (blocks){ // Only report blocks if there are blocks to be reported
      ticks=millis();
      sprintf(buf, "Detected %d:\n", blocks);
      Serial.print(buf);
      for (j=0; j<blocks; j++){
        sprintf(buf, "  block %d: ", j);
        Serial.print(buf); 
        pixy.blocks[j].print();
      }
      /**** Send Data *****/
      Serial.print(F("Now sending..."));
      unsigned long start_time = micros();
      if (!radio.write( &start_time, sizeof(unsigned long))){
        Serial.println(F("No ACK Received...FAILURE"));
      }
      else{
        Serial.print(F("ACK Received...SUCCESS! ("));
        Serial.print(start_time);
        Serial.print(")");
      }
    }
  }
}

void RX(void){
  unsigned long got_time;
  
  // Prevent the radio from listening for data
  radio.startListening();
  
  if( radio.available()){                                       // Variable for the received timestamp
    while (radio.available()){                                  // While there is data ready
      radio.read( &got_time, sizeof(unsigned long));            // Get the payload
      Serial.print("Received Ping: ");
      Serial.println(got_time);
    }
  }
}
