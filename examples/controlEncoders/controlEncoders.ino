/*
  Este ejemplo está probado en un ESP-WROOM-32
  Utilizado 8 encoders, 1 PCF8575 y un pulsador.
  Conectaremos los pines de los encoders a los pines del PCF en parejas (0-1, 2-3, 4-5, ...)
  El pulsador irá al pin 14 del ESP, y el pin INT del PCF al pin 18 del ESP
  Con el pulsador se pondrán todos los valores de los encoders a 0
  En el ejemplo hay un valor máximo y mínimo conún a todos los encoders
  En el monitor serial aparecerá un contador de segundos, y mientras eso está funcionando, podremos 
  mover los encoders apareciendo el número del encoder que se ha movido y su valor
*/

#include <Wire.h>
#include "MUX_PCF857X.h" // https://github.com/MeloCuentan/MUX_PCF857X

void IRAM_ATTR leerPulsadores(); // Leer todas las entradas del PCF8574 y la del interruptor externo
void controlEncoders();          // Control de los encoders
void crearTareas();              // Crear las tareas del sistema
void leerPCF(void *parameter);   // Leer el estado de los pines del PCF8574

const uint8_t ADDR_PCF = 0x20;
const uint8_t cantidadEntradas = 16; // Cantidad de encoders
const uint8_t pinSDA = 23;           // Pin SDA del I2C_OUT
const uint8_t pinSCL = 19;           // Pin SCL del I2C_OUT
const uint8_t pinInterrupcion = 18;  // Pin de interrupción del PCF8574
const uint8_t pinPulsador = 14;      // Pin del pulsador
const uint8_t NUM_ENCODERS = 8;      // Número de encoders

const int8_t VALOR_MINIMO = -100; // Valor mínimo del encoder
const int8_t VALOR_MAXIMO = 100;  // Valor máximo del encoder

volatile uint16_t estadoPins;                          // Estado de los pines del PCF8574
volatile uint8_t lastStates[NUM_ENCODERS] = {0};       // Último estado de los encoders
volatile int8_t encoderSteps[NUM_ENCODERS] = {0};      // Pasos de los encoders
volatile int16_t lastEncoderSteps[NUM_ENCODERS] = {0}; // Últimos pasos de los encoders

volatile bool interrupcionDetectada = false; // Variable para saber si se ha leído el valor del pulsador

uint32_t tiempoActual;           // Variable de tiempo actual del programa
uint32_t tiempoAnterior;         // Variable de tiempo anterior del programa
const uint32_t intervalo = 1000; // Intervalo de tiempo para la lectura de los pulsadores

TaskHandle_t leerPinesPCF;
MUX_PCF pcf(ADDR_PCF, PCF8575, &Wire1); // Multiplexor PCF8575 en la dirección 0x20 y en el puerto I2C adicional

void setup()
{
  Serial.begin(115200);                 // Iniciar puerto serie
  Serial.println("INICIO DEL SISTEMA"); // Mensaje de inicio

  crearTareas(); // Crear las tareas del sistema

  pinMode(pinPulsador, INPUT_PULLUP);                                               // Pin del pulsador como entrada con resistencia de pull-up
  pinMode(pinInterrupcion, INPUT_PULLUP);                                           // Pin de interrupción como entrada con resistencia de pull-up
  attachInterrupt(digitalPinToInterrupt(pinInterrupcion), leerPulsadores, FALLING); // Interrupción para la lectura de los pulsadores

  Wire1.begin(pinSDA, pinSCL, 400000); // Iniciar puerto I2C adicional
  pcf.begin(0xFFFF);                   // Iniciar PCF con todos los pines en estado alto (0xFFFF = 1111 1111 1111 1111)
}

void loop()
{
  tiempoActual = millis();                        // Obtener el tiempo actual
  if (tiempoActual - tiempoAnterior >= intervalo) // Si ha pasado el intervalo de tiempo
  {
    tiempoAnterior = tiempoActual; // Actualizar el tiempo anterior
    Serial.print(millis() / 1000); // Mostrar el tiempo en segundos
    Serial.println("sg");          // Mostrar el texto "sg"
  }

  if (digitalRead(pinPulsador) == LOW) // Si el pulsador se ha presionado
  {
    for (uint8_t i = 0; i < NUM_ENCODERS; i++) // Recorremos todos los encoders
    {
      encoderSteps[i] = 0; // Reiniciar el valor de los encoders
    }
  }

  for (uint8_t i = 0; i < NUM_ENCODERS; i++) // Recorremos todos los encoders
  {
    if (encoderSteps[i] != lastEncoderSteps[i]) // Si el valor del encoder ha cambiado
    {
      lastEncoderSteps[i] = encoderSteps[i]; // Guardamos el último valor del encoder
      Serial.print("Encoder ");              // Mostramos el texto "Encoder"
      Serial.print(i + 1);                   // Mostramos el número del encoder
      Serial.print(": ");                    // Mostramos el texto ": "
      Serial.println(encoderSteps[i]);       // Mostramos el valor del encoder
    }
  }
}

void crearTareas()
{
  xTaskCreatePinnedToCore(
      leerPCF,        // Función de la tarea
      "leerEncoders", // Nombre de la tarea
      10000,          // Tamaño de la pila
      NULL,           // Parámetro de la tarea
      1,              // Prioridad de la tarea
      &leerPinesPCF,  // Manejador de la tarea
      0);             // Núcleo de la tarea (0 o 1)
}

void leerPCF(void *parameter)
{
  for (;;) // Bucle infinito
  {
    if (interrupcionDetectada == true) // Si se ha detectado una interrupción
    {
      interrupcionDetectada = false; // Reiniciamos la variable de interrupción
      controlEncoders();             // Control de los encoders
    }
    vTaskDelay(1); // Retardo de 1 ms
  }
}

void IRAM_ATTR leerPulsadores()
{
  interrupcionDetectada = true; // Indicamos que se ha leído el valor del pulsador
}

void controlEncoders()
{
  uint8_t pinA[NUM_ENCODERS];         // Pines A de los encoders
  uint8_t pinB[NUM_ENCODERS];         // Pines B de los encoders
  uint8_t currentState[NUM_ENCODERS]; // Estado actual de los encoders
  uint8_t lastState[NUM_ENCODERS];    // Último estado de los encoders

  estadoPins = pcf.getPinState(); // Leemos el estado de todos los pines de una vez

  for (uint8_t encoderIndex = 0; encoderIndex < NUM_ENCODERS; encoderIndex++) // Recorremos todos los encoders
  {
    // Leer los pines A y B por separado
    pinA[encoderIndex] = (estadoPins >> (encoderIndex * 2)) & 0b01;     // Desplazamos 0 bits a la derecha y aplicamos una máscara de bits
    pinB[encoderIndex] = (estadoPins >> (encoderIndex * 2 + 1)) & 0b01; // Desplazamos 1 bit a la derecha y aplicamos una máscara de bits

    currentState[NUM_ENCODERS] = (pinA[encoderIndex] << 1) | pinB[encoderIndex]; // Combinamos los estados de A y B en un valor de 2 bits
    lastState[NUM_ENCODERS] = lastStates[encoderIndex];                          // Guardamos el último estado

    if (currentState[NUM_ENCODERS] != lastState[NUM_ENCODERS]) // Verificar si el estado ha cambiado
    {
      if (lastState[NUM_ENCODERS] == 0b01 && currentState[NUM_ENCODERS] == 0b11) // Giro horario: se realiza solo si el centro se alcanza (ambos pines = 1)
      {
        if (encoderSteps[encoderIndex] < VALOR_MAXIMO) // Si el valor del encoder es menor que el máximo
          encoderSteps[encoderIndex]++;                // Incrementar el valor del encoder
      }
      else if (lastState[NUM_ENCODERS] == 0b10 && currentState[NUM_ENCODERS] == 0b11) // Giro antihorario: se realiza solo si el centro se alcanza (ambos pines = 1)
      {
        if (encoderSteps[encoderIndex] > VALOR_MINIMO) // Si el valor del encoder es mayor que el mínimo
          encoderSteps[encoderIndex]--;                // Decrementar el valor del encoder
      }
      lastStates[encoderIndex] = currentState[NUM_ENCODERS]; // Guardar el nuevo estado como el último estado
    }
  }
}
