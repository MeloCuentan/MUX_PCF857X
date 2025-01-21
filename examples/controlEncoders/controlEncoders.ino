/*
  Este ejemplo está probado en un ESP-WROOM-32
  Utilizado 8 encoders, 1 PCF8575 y un pulsador.
  Conectaremos los pines de los encoders a los pines del PCF en parejas (0-1, 2-3, 4-5, ...)
  El pulsador irá al pin 14 del ESP, y el pin INT del PCF al pin 18 del ESP
  Con el pulsador se pondrán todos los valores de los encoders a 0
  En el ejemplo hay un valor máximo y mínimo conún a todos los encoders
  En el monitor serial aparecerá un contador de segundos, y mientras eso está funcionando, podremos 
  mover los encoders apareciendo el número del encoder que se ha movido y su valor
  Se ha mejorado la función controlEncoders y añadido que la tarea se ejecute con la interrupción
*/

#include <Wire.h>
#include <MUX_PCF857X.h> // https://github.com/MeloCuentan/MUX_PCF857X

void IRAM_ATTR ISR();          // Leer todas las entradas del PCF8574 y la del interruptor externo
void controlEncoders();        // Control de los encoders
void crearTareas();            // Crear las tareas del sistema
void leerPCF(void *parameter); // Leer el estado de los pines del PCF8574

const uint8_t ADDR_PCF = 0x20;       // Dirección del PCF8574
const uint8_t cantidadEntradas = 16; // Cantidad de encoders
const uint8_t pinSDA = 23;           // Pin SDA del I2C_OUT
const uint8_t pinSCL = 19;           // Pin SCL del I2C_OUT
const uint8_t pinInterrupcion = 18;  // Pin de interrupción del PCF8574
const uint8_t pinPulsador = 14;      // Pin del pulsador

const uint8_t NUM_ENCODERS = 8;     // Número de encoders
const int8_t VALOR_MINIMO = -100;   // Valor mínimo del encoder
const int8_t VALOR_MAXIMO = 100;    // Valor máximo del encoder
uint8_t pinA[NUM_ENCODERS];         // Pines A de los encoders
uint8_t pinB[NUM_ENCODERS];         // Pines B de los encoders
uint8_t currentState[NUM_ENCODERS]; // Estado actual de los encoders

uint16_t estadoPins;                          // Estado de los pines del PCF8574
uint8_t lastStates[NUM_ENCODERS] = {0};       // Último estado de los encoders
int8_t encoderSteps[NUM_ENCODERS] = {0};      // Pasos de los encoders
int16_t lastEncoderSteps[NUM_ENCODERS] = {0}; // Últimos pasos de los encoders

uint32_t tiempoActual;           // Variable de tiempo actual del programa
uint32_t tiempoAnterior;         // Variable de tiempo anterior del programa
const uint32_t intervalo = 1000; // Intervalo de tiempo para la lectura de los pulsadores

TaskHandle_t leerPinesPCF;              // Manejador de la tarea de lectura de los pines del PCF8574
MUX_PCF pcf(ADDR_PCF, PCF8575, &Wire1); // Multiplexor PCF8575 en la dirección 0x20 y en el puerto I2C adicional

void setup()
{
  Serial.begin(115200);                 // Iniciar puerto serie
  Serial.println("INICIO DEL SISTEMA"); // Mensaje de inicio

  crearTareas(); // Crear las tareas del sistema

  pinMode(pinPulsador, INPUT_PULLUP);                                    // Pin del pulsador como entrada con resistencia de pull-up
  pinMode(pinInterrupcion, INPUT_PULLUP);                                // Pin de interrupción como entrada con resistencia de pull-up
  attachInterrupt(digitalPinToInterrupt(pinInterrupcion), ISR, FALLING); // Interrupción para la lectura de los pulsadores

  Wire1.begin(pinSDA, pinSCL, 400000); // Iniciar puerto I2C adicional
  pcf.begin(0xFFFF);                   // Iniciar PCF con todos los pines en estado alto (0xFFFF = 1111 1111 1111 1111)
}

void loop()
{
  tiempoActual = millis(); // Obtener el tiempo actual

  if (digitalRead(pinPulsador) == LOW) // Si el pulsador se ha presionado
  {
    for (uint8_t i = 0; i < NUM_ENCODERS; i++) // Recorremos todos los encoders
    {
      encoderSteps[i] = 0; // Reiniciar el valor de los encoders
    }
  }

  if (tiempoActual - tiempoAnterior >= intervalo) // Si ha pasado el intervalo de tiempo
  {
    tiempoAnterior = tiempoActual; // Actualizar el tiempo anterior
    Serial.print(millis() / 1000); // Mostrar el tiempo en segundos
    Serial.println("sg");          // Mostrar el texto "sg"

    // for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
    //   Serial.print("Encoder "); // Mostrar el texto "Encoder"
    //   Serial.print(i + 1);      // Mostrar el número del encoder
    //   Serial.print(": ");       // Mostrar el texto ": "
    //   Serial.print(encoderSteps[i]); // Mostrar el valor del encoder
    //   Serial.print(" | ");           // Mostrar el texto " | "
    // }
    // Serial.println(); // Mostrar un salto de línea
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
  xTaskCreate(        // Crear la tarea leerPCF
      leerPCF,        // Función de la tarea
      "LeerPCF",      // Nombre de la tarea
      2048,           // Tamaño de la pila
      NULL,           // Parámetro de la tarea
      1,              // Prioridad de la tarea
      &leerPinesPCF); // Manejador de la tarea
}

void leerPCF(void *parameter)
{
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Espera indefinidamente a la notificación
    controlEncoders();                       // Ejecutar el control de encoders cuando la notificación llegue
  }
}

void IRAM_ATTR ISR()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(leerPinesPCF, &xHigherPriorityTaskWoken); // Notificar a la tarea leerPCF
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);                    // Realizar un cambio de contexto si es necesario
}

void controlEncoders()
{
  estadoPins = pcf.getPinState(); // Leer todos los pines de una vez

  for (uint8_t encoderIndex = 0; encoderIndex < NUM_ENCODERS; encoderIndex++) // Recorrer todos los encoders
  {                                                                           // Calcular los estados de los pines A y B de este encoder
    uint8_t pinOffset = encoderIndex * 2;                                     // Optimizar el cálculo del desplazamiento
    pinA[encoderIndex] = (estadoPins >> pinOffset) & 0b01;                    // Obtener el pin A
    pinB[encoderIndex] = (estadoPins >> (pinOffset + 1)) & 0b01;              // Obtener el pin B

    currentState[encoderIndex] = (pinA[encoderIndex] << 1) | pinB[encoderIndex]; // Combinar en un valor de 2 bits

    if (currentState[encoderIndex] == lastStates[encoderIndex]) // Si el estado no cambió, pasamos al siguiente encoderIndex
      continue;

    switch (lastStates[encoderIndex]) // Estado anterior del encoder
    {
    case 0b01: // Estado anterior: 01 -> Giro en sentido horario
      if (currentState[encoderIndex] == 0b11)
      {
        if (encoderSteps[encoderIndex] < VALOR_MAXIMO) // Si el valor del encoder es menor que el máximo
          encoderSteps[encoderIndex]++;                // Incrementar el valor del encoder
      }
      break;

    case 0b10: // Estado anterior: 10 -> Giro en sentido antihorario
      if (currentState[encoderIndex] == 0b11)
      {
        if (encoderSteps[encoderIndex] > VALOR_MINIMO) // Si el valor del encoder es mayor que el mínimo
          encoderSteps[encoderIndex]--;                // Decrementar el valor del encoder
      }
      break;
    }

    lastStates[encoderIndex] = currentState[encoderIndex]; // Actualizar el último estado
  }
}
