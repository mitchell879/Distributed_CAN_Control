#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>

// Declare stemmaqt Encoder (Manual Test Control)
#define SS_SWITCH 24
#define SS_NEOPIX 6
#define SEESAW_ADDR 0x36
bool Manual_Control = true; // set to false for normal operation

// Declare Variables
#define OPEN_VALVE_COMMAND_PIN 9 // Silkscreen pin 9
#define CLOSE_VALVE_COMMAND_PIN 10 // Silkscreen pin 10
#define OPEN_VALVE_SIGNAL_PIN 6 // Silkscreen pin 6
#define CLOSED_VALVE_SIGNAL_PIN 5 // Silkscreen pin 5
volatile int8_t VALVE_STATE; // 1 = valve open, 0 = valve closed, -1 = valve in motion
volatile bool   VALVE_ERROR = false; // Valve in motion timeout - error
volatile uint32_t VALVE_ERROR_TIME = 0; // Instance time for Valve timeout error
#define VALVE_TRANSITION_TIMEOUT 30000 // Timeout for valve error in ms (default 30,000ms; max 60,000ms)
volatile bool VALVE_COMMAND_OPEN = false;
volatile bool VALVE_COMMAND_CLOSE = false; // Current desired valve state

// Start stemmaqt Encoder objects (for Manual Control)
Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);

// Call if Valve behaves abnormally
void valve_Timeout_Error(uint32_t time){
  // SEND ERROR SIGNAL ; SHUT OFF POWER TO VALVE
}


void setup() {
  ss.begin(SEESAW_ADDR);
  sspixel.begin(SEESAW_ADDR);

  // set not so bright!
  sspixel.setBrightness(20);
  sspixel.show();
  
  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);



  Serial.println("Turning on interrupts");
  delay(10);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();

  // Setup Pins for Valve Control
  pinMode(OPEN_VALVE_COMMAND_PIN, OUTPUT);
  pinMode(CLOSE_VALVE_COMMAND_PIN, OUTPUT);
  pinMode(OPEN_VALVE_SIGNAL_PIN, INPUT);
  pinMode(CLOSED_VALVE_SIGNAL_PIN, INPUT);



}

void loop() {
  // get current uptime
  uint32_t current_time = millis();

  //~~ check for Manual Valve control ~~
  if(Manual_Control){
    if(! ss.digitalRead(SS_SWITCH)){
      if(VALVE_COMMAND_OPEN){
        VALVE_COMMAND_OPEN = false;
        VALVE_COMMAND_CLOSE = true;
        sspixel.Color(127,0,0); // Set Color to red
      }
      else if(VALVE_COMMAND_CLOSE){
        VALVE_COMMAND_CLOSE = false;
        VALVE_COMMAND_OPEN = true;
        sspixel.Color(0,127,0); // Set Color to green
      }
      else{
        // Power on setting
        VALVE_COMMAND_OPEN = true;
        VALVE_COMMAND_CLOSE = false;
        sspixel.Color(0,0,127); // Set Color to blue
      }
    }
    sspixel.show();
  }


  //~~ Check current valve state ~~
  if((digitalRead(OPEN_VALVE_SIGNAL_PIN)==HIGH) && (digitalRead(CLOSED_VALVE_SIGNAL_PIN)==LOW)){
    // OPEN SIGNAL = ON, CLOSED SIGNAL = OFF, therefore valve is open
    VALVE_STATE = 1;
    VALVE_ERROR_TIME = 0;
    VALVE_ERROR = false;
  }
  else if((digitalRead(OPEN_VALVE_SIGNAL_PIN)==LOW) && (digitalRead(CLOSED_VALVE_SIGNAL_PIN)==HIGH)){
    // OPEN SIGNAL = OFF, CLOSED SIGNAL = ON, therefore valve is closed
    VALVE_STATE = 0;
    VALVE_ERROR_TIME = 0;
    VALVE_ERROR = false;
  }
  else{
    // UNCLEAR VALVE STATE, Transition (valve in motion), or Error
    // Check Valve Timeout
    if(((current_time - VALVE_ERROR_TIME) >= VALVE_TRANSITION_TIMEOUT) || VALVE_ERROR){
      valve_Timeout_Error(VALVE_ERROR_TIME);
      VALVE_ERROR_TIME = 0;
      VALVE_ERROR = false;
    }
    else{
      // VALVE Timeout not reached, assume valve in Transition
      VALVE_ERROR_TIME = current_time;
      VALVE_ERROR = false;
    }

    // Set Valve state to unclear, prevent valve command action
    VALVE_STATE = -1;
  }

  // Execute desired valve state
  if(VALVE_COMMAND_OPEN && (VALVE_STATE == 0)){
    // Valve is in closed state & commanded to open
    // Ensure valve close relay is off
    digitalWrite(CLOSE_VALVE_COMMAND_PIN, LOW);
    delay(5);
    // Enable valve open relay
    digitalWrite(OPEN_VALVE_COMMAND_PIN, HIGH);
  }
  if(VALVE_COMMAND_CLOSE && (VALVE_STATE == 1)){
    // Valve is in open state & commanded to close
    // Ensure valve open relay is off
    digitalWrite(OPEN_VALVE_COMMAND_PIN, LOW);
    delay(5);
    // Enable valve close relay
    digitalWrite(CLOSE_VALVE_COMMAND_PIN, HIGH);
  }
  // else valve is in transition to commanded position, or malfunctioning (wait for timeout)
  delay(10); // replace with proper millis() interval later
}
