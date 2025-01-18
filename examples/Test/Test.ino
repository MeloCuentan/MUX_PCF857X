#include "MUX_PCF8575.h"  // Agregamos la librería

const uint8_t ADDR_PCF = 0x20;  // Asignamos la dirección I2C del multiplexor

MUX_PCF8575 pcf(ADDR_PCF);  // Creamos el objeto con la dirección I2C

uint32_t tiempoActual, tiempoAnterior; // Estas variables serán para el control del funcionamiento
uint32_t intervalo = 500;  // Cada vez que se pase ese tiempo, habrá un cambio en los pines de salida
uint8_t numeroPin = 0;

void setup() {
  Serial.begin(115200);

  pcf.begin();  // Inicializamos el objeto

  // *************  comprobar pines como salida ***************
  for (uint8_t i = 0; i < 8; i++) {  // Creamos un bucle para recorrer los pines de P0 a P07
    pcf.pinMode(i, OUTPUT);      // Configurar los pines como salida
  }

  // ************  comprobar pines como entradas **************
  for (uint8_t i = 8; i < 16; i++) {  // Creamos un bucle para recorrer los pines de P10 a P17 (corresponden del 8 al 15)
    pcf.pinMode(i, INPUT_PULLUP);        // Configuramos los pines como entrada
  }
}

void loop() {
  tiempoActual = millis();
  for (uint8_t i = 8; i < 16; i++) {
    if (pcf.digitalRead(i) == LOW) {
      Serial.println("Pulsado el pin " + String(i));
    }
  }

  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    pcf.digitalWrite(numeroPin, !pcf.digitalRead(numeroPin));
    Serial.println(numeroPin);  
    numeroPin++;
    if (numeroPin > 7) numeroPin = 0;     
  }
  delay(200);
}
