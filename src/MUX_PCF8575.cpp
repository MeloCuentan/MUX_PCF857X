#include <MUX_PCF8575.h>
#include "Arduino.h"

MUX_PCF8575::MUX_PCF8575(uint8_t ADDR_I2C, uint16_t estadoInicialPines)
{
  _ADDR_I2C = ADDR_I2C;
  _estadoPines = estadoInicialPines; // Inicialmente, todos los pines están en LOW
}

bool MUX_PCF8575::begin()
{
  // Prueba de comunicación con el dispositivo
  Wire.beginTransmission(_ADDR_I2C);
  delayMicroseconds(10);
  uint8_t result = Wire.endTransmission();

  if (result != 0) return false;  // Retorna false si la inicialización falla

  sendData(0xFFFF);  // Inicializar todos como entrada
  delayMicroseconds(10);
  sendData(_estadoPines);

  return true;  // Retorna true si la inicialización es exitosa
}

void MUX_PCF8575::pinMode(uint8_t pin, uint8_t mode)
{
  if (mode == OUTPUT)
  {
    _estadoPines &= ~(1 << pin); // Establece el pin como salida
  }
  else if (mode == INPUT)
  {
    _estadoPines &= ~(1 << pin); // Desactiva la resistencia pull-up interna
  }
  else if (mode == INPUT_PULLUP)
  {
    _estadoPines |= (1 << pin); // Habilita la resistencia pull-up interna
  }
  sendData(_estadoPines);
}

void MUX_PCF8575::digitalWrite(uint8_t pin, bool value)
{
  if (value == HIGH)
  {
    _estadoPines |= (1 << pin); // Establece el pin en HIGH
  }
  else
  {
    _estadoPines &= ~(1 << pin); // Establece el pin en LOW
  }
  sendData(_estadoPines);
}

void MUX_PCF8575::sendData(uint16_t value)
{
  Wire.beginTransmission(_ADDR_I2C);
  Wire.write(lowByte(value));
  Wire.write(highByte(value));
  Wire.endTransmission();
  delayMicroseconds(10);
}

bool MUX_PCF8575::digitalRead(uint8_t pin)
{
  Wire.beginTransmission(_ADDR_I2C);
  Wire.endTransmission(); // Termina la transmisión y obtiene el resultado
  delayMicroseconds(10);

  Wire.requestFrom(_ADDR_I2C, (uint8_t)2); // Solicita 2 bytes desde la dirección I2C del dispositivo
  delayMicroseconds(10);
  uint16_t data = Wire.read();             // Lee el primer byte
  data |= (Wire.read() << 8);              // Lee el segundo byte y lo desplaza para formar un uint16_t
  return (data >> pin) & 0x01;             // Retorna el estado del pin solicitado
}
