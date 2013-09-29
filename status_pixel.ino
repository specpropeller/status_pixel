// This Arduino Board programm controlls Neopixel LEDs. 
// The colors of the Neopixel are defined via serial commands.
// Application idea:
// It might be usefull to communicate the states of things by attaching a color.
// A PC or Mac is in charge of detecting states and sending coloring commands to
// the Arduino.
// (You can define your own things, states and colors :-) )
//
// requires: Adafruit NeoPixel library
// https://github.com/adafruit/Adafruit_NeoPixel
// To test your neopixel wiring use the strandtest example
//
// Initial Application Tests after flashing THIS program:
// Open Tools / Serial Monitor
// Enter: L00,0F,00,00
// Enter: L01,00,0F,00
// Enter: L02,00,00,0F
// Enter: S
// Expected Result: First LED shows red, second green, third blue 
//
// Known Problems: 
// The show command ("S") is sending many pixelcommands to the neopixels.
// In that time all arduino interrupts are disabled. There has been no
// stress test by now. The caller might be delayed or commands might be
// lost ... I dont know, now


#include <Adafruit_NeoPixel.h>
#define PIN 6 // data connection to the Neopixels
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);



// Connection timeout:
// in some applications it might be better to show no state than a wrong state.
// Code with "CONNECTION_TIMEOUT" will turn all LEDs to black if the the last
// serial command is older than 20 seconds
#ifdef CONNECTION_TIMEOUT
#include "Timer.h" //http://playground.arduino.cc/Code/Timer
Timer t;
int id;
void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  id = t.after(10000, timeout);
}
void timeout() {
  uint8_t i;
  for (i=0;i<8;i++)
    strip.setPixelColor(i,0,0,0);
  strip.show();
}
#else //without CONNECTION_TIMEOUT

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

#endif

// Parser States
// might be simplified by parsing parameter more elegant
// by using 2 States. One for position:
// Command, P1, P2, P3, P4
// And one for parameter parts:
// NONE,First_Nibble, Second_Nibble 

// Now each state reflects one character input
#define ST_NO_STATE 0
#define ST_L_STATE 1 // LED coloring: Begin of the long state names 
#define ST_S_STATE 2
#define ST_L_P1_STATE 3 // P1: First letter of parameter 1 received
#define ST_L_P1X_STATE 4 // P1X: Second letter of P1 received
#define ST_L_P1X_S_STATE 5 // S: Seperator received
#define ST_L_P1X_S_P2_STATE 6 // P2: First letter of parameter 2 received
#define ST_L_P1X_S_P2X_STATE 7 // P2X: You got the idea ;-)
#define ST_L_P1X_S_P2X_S_STATE 8
#define ST_L_P1X_S_P2X_S_P3_STATE 9
#define ST_L_P1X_S_P2X_S_P3X_STATE 10
#define ST_L_P1X_S_P2X_S_P3X_S_STATE 11
#define ST_L_P1X_S_P2X_S_P3X_S_P4_STATE 12
#define ST_L_P1X_S_P2X_S_P3X_S_P4X_STATE 13

// parsing one hex_digit
uint16_t parse_hexnibble(uint16_t input) {
  uint16_t ret = -1;
  if (input>='a' && input<='f') {
    ret = 10 + input - 'a';
  }
  else if (input>='0' && input<='9') {
    ret = input - '0';
  }
  else if (input>='A' && input<='F') {
    ret = 10 + input - 'F';
  }
  return ret;
}

int serialByte = 0; // serial byte from sender  
uint8_t state = 0;  // state of the parser
uint16_t hexnib;    // parsing a character to an hexnible-value
uint8_t p1 = 0;     // collecting Parameter 1
uint8_t p2 = 0;     // collecting Parameter 2 
uint8_t p3 = 0;     // collecting Parameter 3
uint8_t p4 = 0;     // collecting Parameter 4


void loop() {
  
#ifdef CONNECTION_TIMEOUT 
  while (!Serial.available())
    t.update();
  t.stop(id);
  id = t.after(20000, timeout);
#else
  while (!Serial.available());
#endif
  
  serialByte = Serial.read();
  switch (state) {
    case ST_NO_STATE:
      switch (serialByte){
        case 'T':
          // Serial Input "T" ("T"esting):
          // echoes T back to sender. (Could also be used to avoid CONNECTION_TIMEOUT)
          Serial.println("T");
        break;
        case 'L':
          // Serial Input LII,RR,GG,BB:
          // Prepare the Coloring of LED Number II by setting
          // Red (RR), Green (GG) and Blue (BB) to values
          // between 00 to FF (leading zeroes are mandatory)  
          state = ST_L_STATE;
        break;
        case 'S':
          // serial Input S (Show)
          // send all prepared colors to the Neopixel 
          strip.show();
        break;
      }
    break;
    case ST_L_STATE:
      hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1_STATE;
        p1 = hexnib << 4; 
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1_STATE:
      hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_STATE;
        p1 = p1 | hexnib; 
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1X_STATE:
      if (serialByte == ',')
        state++;
      else
        state = ST_NO_STATE;
    break;
    case ST_L_P1X_S_STATE:
      hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_S_P2_STATE;
        p2 = hexnib << 4; 
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1X_S_P2_STATE:
    hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_S_P2X_STATE;
        p2 = p2 | hexnib; 
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1X_S_P2X_STATE:
      if (serialByte == ',')
        state++;
      else
        state = ST_NO_STATE;
    break;
    case ST_L_P1X_S_P2X_S_STATE:
    hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_S_P2X_S_P3_STATE;
        p3 = hexnib << 4; 
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1X_S_P2X_S_P3_STATE:
      hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_S_P2X_S_P3X_STATE;
        p3 = p3 | hexnib; 
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1X_S_P2X_S_P3X_STATE:
      if (serialByte == ',')
        state++;
      else
        state = ST_NO_STATE;
    break;
    case ST_L_P1X_S_P2X_S_P3X_S_STATE:
      hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_S_P2X_S_P3X_S_P4_STATE;
        p4 = hexnib << 4; 
      }
      else {
        state = ST_NO_STATE;
      } 
    break;
    case ST_L_P1X_S_P2X_S_P3X_S_P4_STATE:
      hexnib = parse_hexnibble(serialByte);
      if (hexnib != -1){
        state = ST_L_P1X_S_P2X_S_P3X_S_P4X_STATE;
        p4 = p4 | hexnib; 
        state = ST_NO_STATE;
        strip.setPixelColor(p1,p2,p3,p4);
      }
      else {
        state = ST_NO_STATE;
      }
    break;
    case ST_L_P1X_S_P2X_S_P3X_S_P4X_STATE:
    break;

    default:
    break;
  }
}


