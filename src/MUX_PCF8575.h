#ifndef MUX_PCF8575_h
#define MUX_PCF8575_h

#include <Wire.h>

class MUX_PCF8575 {
public:
  MUX_PCF8575(uint8_t ADDR_I2C, uint16_t estadoInicialPines = 0x0000);

  bool begin();
  void pinMode(uint8_t pin, uint8_t mode);
  void digitalWrite(uint8_t pin, bool value);
  bool digitalRead(uint8_t pin);

private:
  uint8_t _ADDR_I2C;
  uint16_t _estadoPines;
  void sendData(uint16_t value);
};

#endif
