#ifndef MUX_PCF_h
#define MUX_PCF_h

#include <Wire.h>
#include <Arduino.h>

enum MUX_Type  // Tipos de multiplexores
{ 
  PCF8574,
  PCF8575
};

class MUX_PCF
{
public:
  MUX_PCF(uint8_t ADDR_I2C = 0x20, MUX_Type type = PCF8574, TwoWire *wire = &Wire); // Constructor

  bool begin(uint16_t estadoInicialPines = 0x0000); // Iniciar el multiplexor
  void pinMode(uint8_t pin, uint8_t mode);          // Establecer el modo de un pin
  void digitalWrite(uint8_t pin, bool value);       // Escribir en un pin
  bool digitalRead(uint8_t pin);                    // Leer un pin

  uint16_t getPinState();           // Obtener el estado de los pines
  void setPinState(uint16_t value); // Establecer el estado de los pines

private:
  uint8_t _ADDR_I2C;             // Dirección I2C del multiplexor
  uint16_t _estadoPines;         // Estado de los pines
  MUX_Type _type;                // Tipo de multiplexor
  TwoWire *_wire;                // Puntero al bus I2C
  void sendData(uint16_t value); // Enviar datos al multiplexor
};

#endif
