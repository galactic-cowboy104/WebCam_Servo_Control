#include <Servo.h> // Se incluye la librería Servo para controlar los servomotores.

Servo servo1;  // Se declara un objeto Servo para controlar el primer servomotor.
Servo servo2;  // Se declara un objeto Servo para controlar el segundo servomotor.

String command = "";       // Variable para almacenar el comando recibido por Serial.
String lastCommand = "";   // Variable para almacenar el último comando procesado (para evitar procesar comandos repetidos).

void setup() {
  
  Serial.begin(115200);  // Se inicia la comunicación serial a 115200 baudios.

  // Se adjuntan los servos a los pines correspondientes y se definen los límites de pulsos en microsegundos.
  servo1.attach(2, 600, 2500);  // El servomotor 1 se conecta al pin 2, con un rango de pulsos entre 600 y 2500 microsegundos.
  servo2.attach(3, 550, 2520);  // El servomotor 2 se conecta al pin 3, con un rango de pulsos entre 550 y 2520 microsegundos.

  // Se establece la posición inicial de los servos.
  servo1.write(90);  // Se pone el servomotor 1 en 90 grados (posición media).
  servo2.write(0);   // Se pone el servomotor 2 en 0 grados (posición inicial).
  
}

void loop() {
  
  // Se comprueba si hay datos disponibles para leer desde el puerto serie.
  if (Serial.available() > 0) {
    
    // Se lee la cadena de texto hasta el siguiente salto de línea y se almacena en la variable 'command'.
    command = Serial.readStringUntil('\n');
    command.trim();  // Se eliminan espacios y saltos de línea al principio y al final del comando.

    Serial.flush();  // Se limpia el buffer de la comunicación serial.

    // Se verifica si el comando es diferente al último comando procesado.
    if (command != lastCommand) {
      lastCommand = command;  // Se guarda el comando actual como el último comando procesado.
      handleCommand(command);  // Se llama a la función 'handleCommand' para procesar el comando recibido.
    }
    
  }
  
}

// Función que maneja los comandos recibidos.
void handleCommand(String cmd) {
  // Si el comando empieza con "servo1-", se extrae el valor después de "servo1-" y se mueve el servo1 a ese ángulo.
  if (cmd.startsWith("servo1-")) {
    servo1.write(cmd.substring(7).toInt());  // Se toma el valor del comando después de "servo1-" y se mueve el servomotor 1 a esa posición.
  }
  // Si el comando empieza con "servo2-", se extrae el valor después de "servo2-" y se mueve el servo2 a ese ángulo.
  else if (cmd.startsWith("servo2-")) {
    servo2.write(cmd.substring(7).toInt());  // Se toma el valor del comando después de "servo2-" y se mueve el servomotor 2 a esa posición.
  }
}