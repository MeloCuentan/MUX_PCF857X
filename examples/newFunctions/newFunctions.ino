#include <Wire.h>
#include <MUX_PCF857X.h>

void leerPulsadores(); // Leer todas las entradas del PCF8574 y la del interruptor externo

const uint8_t ADDR_PCF = 0x20;
const uint8_t cantidadEntradas = 16;              // Cantidad de encoders
const uint8_t pinSDA = 23;                        // Pin SDA del I2C_OUT
const uint8_t pinSCL = 19;                        // Pin SCL del I2C_OUT
bool valorPulsadores[cantidadEntradas] = {false}; // Valores de los pulsadores

uint32_t tiempoActual;           // Variable de tiempo actual del programa
uint32_t tiempoAnterior;         // Variable de tiempo anterior del programa
const uint32_t intervalo = 1000; // Intervalo de tiempo para la lectura de los pulsadores
uint32_t tiempoInicio;           // Variables para el control del funcionamiento
uint32_t tiempoFinal;            // Variables para el control del funcionamiento

MUX_PCF pcf(ADDR_PCF, PCF8575, &Wire1); // Multiplexor PCF8575 en la dirección 0x20 y en el puerto I2C adicional

void setup()
{
  Serial.begin(115200);                 // Iniciar puerto serie
  Serial.println("INICIO DEL SISTEMA"); // Mensaje de inicio

  Wire1.begin(pinSDA, pinSCL, 400000); // Iniciar puerto I2C adicional
  pcf.begin(0xFFFF);                   // Iniciar PCF con todos los pines en estado alto (0xFFFF = 1111 1111 1111 1111)
}

void loop()
{
  tiempoActual = millis();                        // Iniciamos el contador de tiempo
  if (tiempoActual - tiempoAnterior >= intervalo) // Si ha pasado el tiempo del intervalo
  {
    tiempoAnterior = tiempoActual;                 // Actualizamos el tiempo anterior
    leerPulsadores();                              // Leemos los pulsadores
    for (uint8_t i = 0; i < cantidadEntradas; i++) // Recorremos todos los pulsadores
    {
      Serial.print(i);                  // Mostramos el número del pulsador
      Serial.print(": ");               // Mostramos el número del pulsador
      Serial.print(valorPulsadores[i]); // Mostramos el valor del pulsador
      Serial.print("\t");               // Mostramos el número del pulsador
    }
    Serial.print(tiempoFinal - tiempoInicio); // Mostramos el tiempo que ha tardado en leer los pulsadores
    Serial.println();                         // Salto de línea
  }
}

void leerPulsadores()
{
  tiempoInicio = micros(); // Iniciamos el contador de tiempo

  uint16_t estadoTemp = pcf.getPinState(); // Leemos el estado de todos los pines de una vez

  for (uint8_t i = 0; i < cantidadEntradas; i++) // Recorremos todos los bits
  {
    valorPulsadores[i] = (estadoTemp >> i) & 0x01; // Asignamos el valor del bit a una variable tipo bool
  }
  
  tiempoFinal = micros(); // Finalizamos el contador de tiempo
}
