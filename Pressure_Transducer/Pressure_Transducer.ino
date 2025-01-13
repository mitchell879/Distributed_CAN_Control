/*
 * Adafruit MCP2515 FeatherWing CAN Sender Example
 */
#include <Adafruit_MCP2515.h>
#define CS_PIN    PIN_CAN_CS

#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>
#define SS_SWITCH        24
#define SS_NEOPIX        6

#define SEESAW_ADDR          0x36
Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);

int32_t encoder_position;


// Set CAN bus baud rate
#define CAN_BAUDRATE (250000)

Adafruit_MCP2515 mcp(CS_PIN);

void setup() {
  ss.begin(SEESAW_ADDR);
  sspixel.begin(SEESAW_ADDR);

  // set not so bright!
  sspixel.setBrightness(20);
  sspixel.show();
  
  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);

  // get starting position
  encoder_position = ss.getEncoderPosition();
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();



  mcp.begin(CAN_BAUDRATE);

}

void loop() {
  
  // send packet: id is 11 bits (0x0 - 0x7FF), packet can contain up to 8 bytes of data
  int32_t new_position = analogRead(27);//ss.getEncoderPosition();
  //mcp lib can only write 8 bits at a time, so decompose 32-bit int to 4 8-bit ints
  uint8_t a = new_position & 0xff;
  uint8_t b = (new_position >> 8) & 0xff;
  uint8_t c = (new_position >> 16) & 0xff;
  uint8_t d = (new_position >> 24) & 0xff;

  if(new_position != encoder_position){
    sspixel.setPixelColor(0, Wheel(new_position & 0xFF));
    sspixel.show();

    mcp.beginPacket(0x1);
    mcp.write('P');
    mcp.write(a);
    mcp.write(b);
    mcp.write(c);
    mcp.write(d);
    mcp.endPacket();

    encoder_position = new_position;
    delay(500);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return sspixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return sspixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return sspixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
