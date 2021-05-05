/* Simple buttonbox sketch
 *
 * Supports up to 25 buttons and up to 4 encoders (if encoders have capabilities, they count also as one of the 25 buttons if connected properly)
 *
 * Version 0.1 by Rhundesign, based on the work done by XSimulator user TOPMO3
 *
 *
 * Requirements
 * Arduino IDE 1.6.6 (or above)
 * Joystick library from  Matthew Heironimus, https://github.com/MHeironimus/ArduinoJoystickLibrary
 */

/* Includes */
#include <Keypad.h>
#include <Joystick.h>

/* Variable definition */
#define ENABLE_PULLUPS
#define NUMROTARIES 4
#define NUMBUTTONS 25
#define NUMROWS 5
#define NUMCOLS 5

/* SIMPLE BUTTONS */
/* Define the matrix for the on/off buttons */
byte buttons[NUMROWS][NUMCOLS] = {
  {0,1,2,3,4}, // row 1
  {5,6,7,8,9}, // row 2
  {10,11,12,13,14}, // row 3
  {15,16,17,18,19}, // row 4
  {20,21,22,23,24}, // row 5
};

/* Define the PCB pins where this matrix will be connected */
byte rowPins[NUMROWS] = {21,20,19,18,15}; // define row pins
byte colPins[NUMCOLS] = {14,16,10,9,8}; // define column pins

/* ROTARY ENCODERS */
/* Define the rotary encoder object structure */
struct rotariesdef {
  byte pin1; // pin 1
  byte pin2; // pin 2
  int ccwchar; // counter clockwise virtual button
  int cwchar; // clockwise virtual button
  volatile unsigned char state; // encoder state
};

/* Define the four rotary encoders */
rotariesdef rotaries[NUMROTARIES] {
  {0,1,26,27,0}, // encoder 1 values
  {2,3,28,29,0}, // encoder 2 values
  {4,5,30,31,0}, // encoder 3 values
  {6,7,32,33,0}, // encoder 4 values
};

/* Define encoder directions and states */
#define DIR_CCW 0x10
#define DIR_CW 0x20
#define R_START 0x0

#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START}, // R_START
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW}, // R_CW_FINAL
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START}, // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START}, // R_CW_NEXT
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START}, // R_CCW_BEGIN
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW}, // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START}, // R_CCW_NEXT
};

/* JOYSTICK */
/* Initialize an instance of class Keypad */
Keypad buttbx = Keypad( makeKeymap(buttons), rowPins, colPins, NUMROWS, NUMCOLS); 

/* Initialize a Joystick controller with 34 buttons and no axis */
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
  34, // button count
  0, // Hat switch (POV) count
  false, // X axis
  false, // Y azis
  false, // Z axis
  false, // Rx axis
  false, // Ry axis
  false, // Rz axis
  false, // Rudder axis
  false, // Throttle axis
  false, // Accelerator axis
  false, // Brake axis
  false); // Steering axis

/* START FUNCTIONING */

void setup() {
  Joystick.begin();
  rotary_init();
}

void loop() { 
  CheckAllEncoders();
  CheckAllButtons();
}

void rotary_init() {
  for (int i=0;i<NUMROTARIES;i++) {
    pinMode(rotaries[i].pin1, INPUT);
    pinMode(rotaries[i].pin2, INPUT);
    #ifdef ENABLE_PULLUPS
      digitalWrite(rotaries[i].pin1, HIGH);
      digitalWrite(rotaries[i].pin2, HIGH);
    #endif
  }
}

/* Function to check encoder states */
void CheckAllEncoders(void) {
  for (int i=0;i<NUMROTARIES;i++) {
    unsigned char result = rotary_process(i);
    if (result == DIR_CCW) {
      Joystick.setButton(rotaries[i].ccwchar, 1); delay(50); Joystick.setButton(rotaries[i].ccwchar, 0);
    };
    if (result == DIR_CW) {
      Joystick.setButton(rotaries[i].cwchar, 1); delay(50); Joystick.setButton(rotaries[i].cwchar, 0);
    };
  }
}

/* Function to check button states */
void CheckAllButtons(void) {
  if (buttbx.getKeys()) {
    for (int i=0; i<LIST_MAX; i++) {   // Scan the whole key list.
      if ( buttbx.key[i].stateChanged ) {   // Only find keys that have changed state.
        switch (buttbx.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
          case HOLD:
            Joystick.setButton(buttbx.key[i].kchar, 1);
            break;
          case RELEASED:
          case IDLE:
            Joystick.setButton(buttbx.key[i].kchar, 0);
            break;
        }
      }   
    }
  }
}

/* Function to assign encoder states */
unsigned char rotary_process(int _i) {
  unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
  rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];
  return (rotaries[_i].state & 0x30);
}
