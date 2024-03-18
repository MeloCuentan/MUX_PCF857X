#include "MUX_PCF8575.h"
#include "Arduino.h"

MUX_PCF8575::MUX_PCF8575(uint8_t ADDR_I2C, uint16_t estadoInicialPines) {
  _ADDR_I2C = ADDR_I2C;
  _estadoPines = estadoInicialPines;  // Inicialmente, todos los pines están en LOW
}

void MUX_PCF8575::begin() {
  Wire.begin();
  sendData(0xFFFF); // Inicializar todos como entrada
  sendData(_estadoPines);
}

void MUX_PCF8575::pinMode(uint8_t pin, uint8_t mode) {
  if (mode == OUTPUT) {
    _estadoPines &= ~(1 << pin); // Establece el pin como salida

  } else if (mode == INPUT) {
    _estadoPines &= ~(1 << pin); // Desactiva la resistencia pull-up interna
  } else if (mode == INPUT_PULLUP) {
    _estadoPines |= (1 << pin); // Habilita la resistencia pull-up interna
  }
  sendData(_estadoPines);
}

void MUX_PCF8575::digitalWrite(uint8_t pin, bool value) {
  if (value == HIGH) {
    _estadoPines |= (1 << pin);  // Establece el pin en HIGH
  } else {
    _estadoPines &= ~(1 << pin);  // Establece el pin en LOW
  }
  sendData(_estadoPines);
}

void MUX_PCF8575::sendData (uint16_t value) {
  Wire.beginTransmission(_ADDR_I2C);
  Wire.write(lowByte(value));
  Wire.write(highByte(value));
  Wire.endTransmission();
}

bool MUX_PCF8575::digitalRead(uint8_t pin) {
  Wire.beginTransmission(_ADDR_I2C);
  Wire.requestFrom(_ADDR_I2C, (uint8_t)2);
  uint16_t data = Wire.read();
  data |= (Wire.read() << 8);
  return (data >> pin) & 0x01;
}
