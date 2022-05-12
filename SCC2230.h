#ifdef Software
#include <SoftSPI.h>
#else
#include <SPI.h>
#endif
#ifndef __AVR__
#include <limits>
#endif

// Standard read requests
#define READ_RATE 0x040000f7
#define READ_ACC_X 0x100000e9
#define READ_ACC_Y 0x140000ef
#define READ_ACC_Z 0x180000e5
#define READ_TEMP 0x1c0000e3
#define READ_STAT_SUM 0x7c0000b3
#define READ_RATE_STAT1  0x240000c7
#define READ_RATE_STAT2  0x280000cd
#define READ_ACC_STAT  0x3c0000d3
#define READ_COM_STAT1  0x6c0000ab
// Standard write requests
#define WRITE_FLT_60  0xfc200006
#define WRITE_FLT_10 0xfc1000c7

class Magnet {

  private:
    #ifdef Software
    SoftSPI *mySPI = NULL;
    #else
    SPIClass *hspi = NULL;
    #endif
    char pin_SS1, pin_RST;
    uint32_t firstHalf_request = 0, secondHalf_request = 0;
    uint32_t firstHalf_read = 0, secondHalf_read = 0;

  public:
    double RATE, ACC_X, ACC_Y, ACC_Z, TEMP, STAT_SUM, RATE_STAT1, RATE_STAT2, ACC_STAT, COM_STAT1;
    void begin(char pin_MOSI, char pin_MISO, char pin_SLCK, char _pin_SS1, char _pin_RST) {
      pinMode(pin_MOSI, OUTPUT);
      pinMode(pin_MISO, INPUT);
      pinMode(pin_SLCK, OUTPUT);
      this->pin_SS1 = _pin_SS1;
      this->pin_RST = _pin_RST;
      pinMode(this->pin_SS1, OUTPUT);
      digitalWrite(this->pin_SS1, HIGH); // Turn off the SS
      pinMode(this->pin_RST, OUTPUT);
      digitalWrite(this->pin_RST, HIGH); // Turn off the Reset pin
      #ifdef Software
      this->mySPI = new SoftSPI(pin_MOSI, pin_MISO, pin_SLCK);
      this->mySPI->setClockDivider(SPI_CLOCK_DIV2); // default
      this->mySPI->setBitOrder(MSBFIRST); // default
      this->mySPI->setDataMode(SPI_MODE0); // default
      this->mySPI->begin();
      #else
      #ifndef ARDUINO_TEENSY40 // also Teensy 3.X nad 4.1
      this->hspi = new SPIClass();
      #endif
      #if defined(__AVR__) || defined(ARDUINO_TEENSY40) // also Teensy 3.X nad 4.1
      this->hspi->begin(); // just for Arduino Leonardo
      #else
      this->hspi->begin(this->pin_SS1); // for e.g. STM32 series
      #endif
      #endif
    }
    
    template<typename T, size_t N>
    void ReadData(T (&arr)[N]) {
      #ifndef Software
      this->hspi->beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
      #endif
      for (uint8_t i = 0; i < N+1; i++) { // N+1 due to first reading is old
        if (i < N) {
          this->firstHalf_request = (uint32_t)(arr[i] >> 16);
          this->secondHalf_request = (uint32_t)(arr[i] & 0xffff);
        }
        else this->firstHalf_request = 0, this->secondHalf_request = 0;
        digitalWrite( this->pin_SS1, LOW); // Select
        #ifdef Software
        this->firstHalf_read = mySPI->transfer16(firstHalf_request);
        #else
        this->firstHalf_read = hspi->transfer16(firstHalf_request);
        #endif
        #ifdef Software
        this->secondHalf_read = mySPI->transfer16(secondHalf_request);
        #else
        this->secondHalf_read = hspi->transfer16(secondHalf_request);
        #endif
        digitalWrite( this->pin_SS1, HIGH); // Deselect
        // convert to int32_t and inverse bit (sensor sends inverse logic)
        int32_t Reads = ((~firstHalf_read << 16) | (~secondHalf_read & 0xffff));
        // split to the 4 times bytes parts 
        int8_t lit_int[4];
        lit_int[0] = (int8_t)(Reads >>  0); // CRC ?????????????
        lit_int[1] = (int8_t)(Reads >>  8);
        lit_int[2] = (int8_t)(Reads >> 16);
        lit_int[3] = (int8_t)(Reads >> 24); // ID ??????????????
        Reads = ((lit_int[2] << 8) | (lit_int[1] & 0xff)); // get readings
        Reads = static_cast<int32_t>(Reads); // fix the sign problem // Reads = ?????????
        if (arr[i-1] == READ_RATE) RATE = ((float)Reads/50.0)/5.0;
        if (arr[i-1] == READ_ACC_X) ACC_X = (float)Reads/5886.0;
        if (arr[i-1] == READ_ACC_Y) ACC_Y = (float)Reads/5886.0;
        if (arr[i-1] == READ_ACC_Z) ACC_Z = (float)Reads/5886.0;
        if (arr[i-1] == READ_TEMP) TEMP = 60.0 + (((-1.0)*(float)Reads)/(float)14.7);
        //if (arr[i-1] == READ_STAT_SUM)
        //if (arr[i-1] == READ_RATE_STAT1)
        //if (arr[i-1] == READ_RATE_STAT2)
        //if (arr[i-1] == READ_ACC_STAT)
        //if (arr[i-1] == READ_COM_STAT1)
      }
      #ifndef Software
      this->hspi->endTransaction();
      #endif
    }

    void Reset() {
      digitalWrite(this->pin_RST, LOW);
      delayMicroseconds(100);
      digitalWrite(this->pin_RST, HIGH);
    }
};
