#include "MUX_PCF857X.h"
#include "Arduino.h"

/**
 * @brief Constructor de la clase MUX_PCF
 * @param ADDR_I2C Dirección I2C del multiplexor
 */
MUX_PCF::MUX_PCF(uint8_t ADDR_I2C, MUX_Type type, TwoWire *wire)
{
  _ADDR_I2C = ADDR_I2C; // Establece la dirección I2C
  _type = type;         // Establece el tipo de multiplexor
  _wire = wire;         // Asigna el bus I2C especificado
}

/**
 * @brief Método para iniciar el multiplexor
 * @param estadoInicialPines Estado inicial de los pines
 * @return Verdadero si la transmisión fue exitosa
 */
bool MUX_PCF::begin(uint16_t estadoInicialPines)
{
  _estadoPines = (_type == PCF8574) ? (estadoInicialPines & 0x00FF) : estadoInicialPines; // Establece el estado inicial de los pines
  uint8_t result = sendData(_estadoPines);                                                // Envía el estado de los pines
  return result == 0;                                                                     // Devuelve verdadero si la transmisión fue exitosa
}

/**
 * @brief Método para establecer el modo de un pin
 * @param pin Número del pin
 * @param mode Modo del pin (INPUT, OUTPUT, INPUT_PULLUP)
 */
void MUX_PCF::pinMode(uint8_t pin, uint8_t mode)
{
  if (mode == OUTPUT)                             // Si el modo es de salida
    _estadoPines &= ~(1 << pin);                  // Establece el pin como salida
  else if (mode == INPUT || mode == INPUT_PULLUP) // Si el modo es de entrada
    _estadoPines |= (1 << pin);                   // Establece el pin como entrada
  sendData(_estadoPines);                         // Envía el estado de los pines
}

/**
 * @brief Método para escribir en un pin
 * @param pin Número del pin
 * @param value Valor a escribir en el pin
 */
void MUX_PCF::digitalWrite(uint8_t pin, bool value)
{
  if (value == HIGH)             // Si el valor es alto
    _estadoPines |= (1 << pin);  // Establece el pin como alto
  else                           // Si el valor es bajo
    _estadoPines &= ~(1 << pin); // Establece el pin como bajo
  sendData(_estadoPines);        // Envía el estado de los pines
}

/**
 * @brief Método para leer un pin
 * @param pin Número del pin
 * @return Valor del pin
 */
bool MUX_PCF::digitalRead(uint8_t pin)
{
  _wire->beginTransmission(_ADDR_I2C);                                // Inicia la transmisión
  _wire->endTransmission();                                           // Finaliza la transmisión
  _wire->requestFrom(_ADDR_I2C, (uint8_t)(_type == PCF8575 ? 2 : 1)); // Solicita la lectura de los pines

  uint16_t data = _wire->read();  // Lee el byte menos significativo
  if (_type == PCF8575)           // Si es un PCF8575
    data |= (_wire->read() << 8); // Lee el byte más significativo
  return (data >> pin) & 0x01;    // Devuelve el valor del pin solicitado
}

/**
 * @brief Método para obtener el estado de los pines
 * @return Estado de los pines
 */
uint16_t MUX_PCF::getPinState()
{
  _wire->beginTransmission(_ADDR_I2C);                                // Inicia la transmisión
  _wire->endTransmission();                                           // Finaliza la transmisión
  _wire->requestFrom(_ADDR_I2C, (uint8_t)(_type == PCF8575 ? 2 : 1)); // Solicita la lectura de los pines

  uint16_t data = _wire->read();                      // Lee el byte menos significativo
  if (_type == PCF8575)                               // Si es un PCF8575
    data |= (_wire->read() << 8);                     // Lee el byte más significativo
  return (_type == PCF8574) ? (data & 0x00FF) : data; // Devuelve el estado de los pines
}

/**
 * @brief Método para establecer el estado de los pines
 * @param value Estado de los pines
 */
void MUX_PCF::setPinState(uint16_t value) // Método para establecer el estado de los pines
{
  _estadoPines = (_type == PCF8574) ? (value & 0x00FF) : value; // Establece el estado de los pines
  sendData(_estadoPines);                                       // Envía el estado de los pines
}

/**
 * @brief Método privado para enviar datos al multiplexor
 * @param value Valor a enviar
 * @return Resultado de la transmisión
 */
uint8_t MUX_PCF::sendData(uint16_t value)
{
  _wire->beginTransmission(_ADDR_I2C); // Inicia la transmisión
  _wire->write(lowByte(value));        // Envía el byte menos significativo
  if (_type == PCF8575)                // Si es un PCF8575
    _wire->write(highByte(value));     // Envía el byte más significativo
  return _wire->endTransmission();     // Finaliza la transmisión
}
