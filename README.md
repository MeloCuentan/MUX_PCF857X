# Multiplexor_PCF8575
Uso básico del multiplexor PCF8575

Librería creada para utilizar el multiplexor de la forma más sencilla posible.
Los métodos son iguales al uso habitual del entonro de Arduino (pinMode, digitalRead, digitalWrite).

Se han añadido algunos métodos adicionales y modificado el constructor
CONSTRUCTOR:
  - MUX_PCF(uint8_t ADDR_I2C = 0x20, MUX_Type type = PCF8574, TwoWire *wire = &Wire);
    Se asignan valores predeterminados, pero así se puede crear no solo con varias direcciones sino poder seleccionar si el tipo y el canal I2C que se va a utilizar
   
METODOS ADICIONALES:
  - uint16_t getPinState();
   Este método devuelve el estado de todos los pines en un uint16_t consiguiendo tener una sola lectura a poder agilizar la velocidad para el resto del código
   En caso de estar utilizando un PCF8574, los valores serán de los dos bytes menos significativos.
  
  - void setPinState(uint16_t value);
    Este método recibe el valor que queramos tener en todos los pines. Se hará un solo envío lo que requerirá menor tiempo.
    En caso de utilizar un PCF8474 solo se utilizarán los dos bytes menos significativos.
