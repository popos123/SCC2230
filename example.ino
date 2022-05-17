//#define Software // software if definde or hardware SPI if not
#include <Arduino.h>
#include "SCC2230.h"

uint32_t REQ_SEND[] = {READ_RATE, READ_ACC_X, READ_ACC_Y, READ_ACC_Z, READ_TEMP};
uint32_t REQ_SET[] = {WRITE_FLT_60};

Magnet SCC2230;

void setup() {
  Serial.begin(1000000); // init serial comm at 1'000'000
  SCC2230.begin(13, 12, 15, 14, 39); // MOSI, MISO, SCK, CS, RST
  SCC2230.ReadData(REQ_SET); // set init config
}

void loop() {
  SCC2230.ReadData(REQ_SEND); // read data
  Serial.println(SCC2230.RATE + String(" ") + SCC2230.ACC_X + String(" ") + SCC2230.ACC_Y + String(" ") + SCC2230.ACC_Z + String(" ") + SCC2230.TEMP);
  delay(100);
}
