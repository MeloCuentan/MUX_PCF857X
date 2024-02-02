#ifndef MUX_PCF8575_h
#define MUX_PCF8575_h

#include <Wire.h>

class MUX_PCF8575 {
public:
  MUX_PCF8575(uint8_t ADDR_I2C, uint16_t estadoInicialPines);

  void begin();
  void pinMode(uint8_t pin, uint8_t mode);
  void digitalWrite(uint8_t pin, bool value);
  uint8_t digitalRead(uint8_t pin);

private:
  uint8_t _ADDR_I2C;
  uint16_t _estadoPines;
  uint16_t _estadoSalida;
};

#endif
