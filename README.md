# WebCam Servo Control

Este proyecto permite controlar en tiempo real dos servomotores mediante una interfaz web y visualizar una transmisión de video desde una ESP32-CAM. Se compone de dos partes principales: el servidor alojado en la ESP32-CAM y el cliente basado en un Arduino Nano. Además, incluye archivos STL para imprimir soportes físicos que complementan la instalación del sistema.

## Materiales Necesarios

- **ESP32-CAM**: Microcontrolador que actúa como servidor y transmite el video.
- **Arduino Nano**: Controlador que recibe los comandos de los servos desde la ESP32-CAM.
- **Servos SG90**: Dos unidades para movimiento controlado.
- **Capacitor de 1000 µF**: Para estabilizar el voltaje de los servomotores.
- **Resistores de 10 kΩ**: Tres unidades para el circuito de la ESP32-CAM.
- **Fuente de alimentación de 5V**: Con un amperaje adecuado para alimentar el circuito y los servomotores.
- **Cables de conexión**: Para unir los componentes electrónicos.

## Contenido del Repositorio

1. **Código del Servidor (Server)**: Código de la ESP32-CAM que establece el servidor HTTP, controla la transmisión de video y envía los comandos de los servomotores.
2. **Código del Cliente (Client)**: Código del Arduino Nano que interpreta los comandos enviados por la ESP32-CAM y controla los servos.
3. **Mallas**: Carpeta que contiene los archivos STL para imprimir las bases de los servomotores y el soporte de la cámara.
4. **Diagrama de Conexiones**: Archivo gráfico que muestra cómo conectar los componentes electrónicos del proyecto.

## Instrucciones de Uso

### Configuración del Servidor (ESP32-CAM):
1. Carga el código "Server" en la ESP32-CAM utilizando el IDE de Arduino.
2. Configura las credenciales de WiFi en el archivo del servidor (reemplaza `ssid` y `password` con tu red).
3. Alimenta la ESP32-CAM con la fuente de 5V.

### Configuración del Cliente (Arduino Nano):
1. Carga el código "Client" en el Arduino Nano utilizando el IDE de Arduino.
2. Conecta los pines de control de los servos a los pines especificados en el código.

### Conexiones Físicas:
1. Sigue el diagrama de conexiones incluido en el repositorio.
2. Asegúrate de conectar correctamente los resistores de 10 kΩ y el capacitor de 1000 µF en los puntos indicados.
3. Alimenta el circuito utilizando la fuente de 5V con un amperaje adecuado para los servos.

### Uso del Sistema:
1. Enciende el sistema y espera a que la ESP32-CAM muestre la dirección IP en el monitor serie.
2. Accede a la dirección IP desde un navegador web conectado a la misma red.
3. Utiliza la interfaz web para controlar los servos y visualizar la transmisión de video.

## Consejos Adicionales

- Asegúrate de que la fuente de alimentación tenga suficiente capacidad para evitar caídas de voltaje.
- Imprime los soportes incluidos en la carpeta "Mallas" para un montaje estable de los servos y la cámara.
- Verifica las conexiones antes de alimentar el circuito para evitar daños en los componentes.

## Créditos

Este proyecto fue diseñado para aprender y experimentar con control remoto en tiempo real de hardware embebido, integrando comunicación serial, servidores HTTP y transmisión de video.

¡Disfruta explorando las posibilidades de **WebCam Servo Control**!
