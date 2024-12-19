#include "esp_camera.h"       // Biblioteca para la funcionalidad de la cámara ESP32
#include <WiFi.h>             // Biblioteca para la conectividad Wi-Fi
#include "esp_timer.h"        // Biblioteca para manejo de temporizadores
#include "img_converters.h"   // Biblioteca para convertir formatos de imágenes
#include "fb_gfx.h"           // Biblioteca para gráficos framebuffer
#include "soc/soc.h"          // Biblioteca para acceso a los registros del SoC
#include "soc/rtc_cntl_reg.h" // Biblioteca para controlar funciones del RTC
#include "esp_http_server.h"  // Biblioteca para crear un servidor HTTP en el ESP32

// Credenciales Wi-Fi (se deben completar antes de cargar el código)
const char* ssid = "";        // Nombre de la red Wi-Fi
const char* password = "";    // Contraseña de la red Wi-Fi

// Definición del delimitador para datos en streaming
#define PART_BOUNDARY "123456789000000000000987654321"

// Selecciona el modelo de cámara utilizado (AI Thinker en este caso)
#define CAMERA_MODEL_AI_THINKER

// Configuración de pines para la cámara AI Thinker
#define PWDN_GPIO_NUM     32  // Pin para habilitar/deshabilitar energía de la cámara
#define RESET_GPIO_NUM    -1  // Pin de reinicio de la cámara (no se usa aquí)
#define XCLK_GPIO_NUM      0  // Pin del reloj externo (XCLK)
#define SIOD_GPIO_NUM     26  // Pin de datos I2C (SDA)
#define SIOC_GPIO_NUM     27  // Pin de reloj I2C (SCL)
#define Y9_GPIO_NUM       35  // Pin de datos D9 (MSB del bus de datos de la cámara)
#define Y8_GPIO_NUM       34  // Pin de datos D8
#define Y7_GPIO_NUM       39  // Pin de datos D7
#define Y6_GPIO_NUM       36  // Pin de datos D6
#define Y5_GPIO_NUM       21  // Pin de datos D5
#define Y4_GPIO_NUM       19  // Pin de datos D4
#define Y3_GPIO_NUM       18  // Pin de datos D3
#define Y2_GPIO_NUM        5  // Pin de datos D2 (LSB del bus de datos de la cámara)
#define VSYNC_GPIO_NUM    25  // Pin de sincronización vertical (VSYNC)
#define HREF_GPIO_NUM     23  // Pin de referencia horizontal (HREF)
#define PCLK_GPIO_NUM     22  // Pin de reloj de píxeles (PCLK)

// Pin para controlar el LED del flash
#define FLASH_LED_PIN 4       // Pin para activar/desactivar el LED integrado

// Constantes para definir el tipo de contenido y formato de streaming
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Manejadores para los servidores HTTP
httpd_handle_t camera_httpd = NULL; // Servidor HTTP para la cámara
httpd_handle_t stream_httpd = NULL; // Servidor HTTP para el streaming

// Página HTML para la interfaz del servidor
static const char PROGMEM INDEX_HTML[] = R"rawliteral(

  <!DOCTYPE html>
  <html lang="es-MX">
  
      <head>
  
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  
          <title>ESP32-CAM</title>
  
          <style>
              
              body {
                  background-color: #B6B1B1;
                  font-family: Arial, Helvetica, sans-serif;
                  margin: 0;
                  padding: 0;
              }
  
              header {
                  text-align: center;
                  padding: 10px;
                  background-color: #333;
              }
  
              h1 {
                  color: #F7EFEF;
                  font-size: 2.7rem;
                  -webkit-touch-callout: none;
                  user-select: none;
                  -webkit-tap-highlight-color: transparent;
              }
  
              .main-container {
                  display: flex;
                  justify-content: center;
                  align-items: center;
                  padding: 20px;
                  height: 70vh;
              }
  
              .container {
                  display: flex;
                  flex-direction: column;
                  align-items: center;
                  justify-content: center;
                  margin: 0 20px;
              }
  
              .image-container img {
                  max-width: 100%;
                  height: auto;
                  border: 2px solid #333;
                  border-radius: 15px;
                  box-shadow: 0px 6px 10px rgba(0, 0, 0, 0.3);
              }
  
              .controls-container {
                  text-align: center;
              }
  
              .switch {
                  position: relative;
                  display: inline-block;
                  width: 60px;
                  height: 34px;
              }
  
              .switch input {
                  opacity: 0;
                  width: 0;
                  height: 0;
              }
  
              .slider {
                  position: absolute;
                  cursor: pointer;
                  top: 0;
                  left: 0;
                  right: 0;
                  bottom: 0;
                  background-color: #ccc;
                  transition: 0.4s;
                  border-radius: 34px;
              }
  
              .slider:before {
                  position: absolute;
                  content: "";
                  height: 26px;
                  width: 26px;
                  border-radius: 50%;
                  left: 4px;
                  bottom: 4px;
                  background-color: white;
                  transition: 0.4s;
              }
  
              input:checked + .slider {
                  background-color: #4CAF50;
              }
  
              input:checked + .slider:before {
                  transform: translateX(26px);
              }
  
              .slider-text {
                  font-size: 20px;
                  margin-top: 10px;
                  -webkit-touch-callout: none;
                  user-select: none;
                  -webkit-tap-highlight-color: transparent;
              }
  
              .slider-container {
                  margin: 15px 0;
                  text-align: center;
              }
  
              input[type="range"] {
                  -webkit-appearance: none;
                  appearance: none;
                  width: 325px;
                  height: 16px;
                  border-radius: 20px;
                  background: #C9EED5;
                  outline: none;
                  cursor: pointer;
                  transition: background 0.3s ease;
              }
  
              input[type="range"]:hover {
                  background: #B0D8BF;
              }
  
              input[type="range"]::-webkit-slider-thumb {
                  -webkit-appearance: none;
                  appearance: none;
                  width: 20px;
                  height: 20px;
                  background: #15B838;
                  border-radius: 50%;
                  box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.3);
                  cursor: pointer;
                  transition: transform 0.2s ease, background 0.3s ease;
              }
  
              input[type="range"]::-webkit-slider-thumb:active {
                  transform: scale(1.2);
                  background: #0A8D27;
              }
  
              input[type="range"]::-moz-range-thumb {
                  width: 20px;
                  height: 20px;
                  background: #15B838;
                  border-radius: 50%;
                  box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.3);
                  cursor: pointer;
                  transition: transform 0.2s ease, background 0.3s ease;
              }
  
              input[type="range"]::-moz-range-thumb:active {
                  transform: scale(1.2);
                  background: #0A8D27;
              }
  
              input[type="range"]::-ms-thumb {
                  width: 20px;
                  height: 20px;
                  background: #15B838;
                  border-radius: 50%;
                  box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.3);
                  cursor: pointer;
                  transition: transform 0.2s ease, background 0.3s ease;
              }
  
              input[type="range"]::-ms-thumb:active {
                  transform: scale(1.2);
                  background: #0A8D27;
              }
  
              input[type="range"]::-webkit-slider-runnable-track {
                  height: 16px;
                  background: #C9EED5;
                  border-radius: 20px;
              }
  
              input[type="range"]::-moz-range-track {
                  height: 16px;
                  background: #C9EED5;
                  border-radius: 20px;
              }
  
              input[type="range"]::-ms-track {
                  width: 100%;
                  height: 16px;
                  background: transparent;
                  border-color: transparent;
                  color: transparent;
              }
  
              input[type="range"]::-ms-fill-lower {
                  background: #C9EED5;
                  border-radius: 20px;
              }
  
              input[type="range"]::-ms-fill-upper {
                  background: #C9EED5;
                  border-radius: 20px;
              }
  
              .angle {
                  font-size: 20px;
                  font-weight: bold;
                  margin-top: 0;
                  -webkit-touch-callout: none;
                  user-select: none;
                  -webkit-tap-highlight-color: transparent;
              }
  
              @media (max-width: 900px) {
  
                  .main-container {
                      flex-direction: column;
                      align-items: stretch;
                      justify-content: space-between;
                      height: auto;
                  }
  
                  header {
                      padding: 4px;
                  }
  
                  h1 {
                      color: #F7EFEF;
                      font-size: 2.2rem;
                  }
  
                  .image-container img {
                      width: 92%;
                  }
  
                  .container {
                      margin: 10px 0;
                  }
  
                  .slider-container {
                      width: 65%;
                  }
  
                  input[type="range"] {
                      width: 90%;
                  }
  
              }
  
          </style>
  
      </head>
  
      <body>
  
          <header>
              <h1>ESP32-CAM</h1>
          </header>
  
          <div class="main-container">
  
              <div class="container image-container">
                  <img src="" id="photo" alt="Camera Stream">
              </div>
  
              <div class="container controls-container">
  
                  <label class="switch">
                      <input type="checkbox" id="flashSwitch" onchange="toggleFlash()">
                      <span class="slider"></span>
                  </label>
                  <p class="slider-text" id="flashText">Flash: Apagado</p>
  
                  <div class="slider-container">
                      <input type="range" id="angleSlider1" min="0" max="180" value="0" step="15" oninput="updateAngle(this.value, 'angleValue1')" />
                      <p class="angle" id="angleValue1">Servo 1: 90°</p>
                  </div>
  
                  <div class="slider-container">
                      <input type="range" id="angleSlider2" min="0" max="90" value="0" step="10" oninput="updateAngle(this.value, 'angleValue2')" />
                      <p class="angle" id="angleValue2">Servo 2: 0°</p>
                  </div>
  
              </div>
  
          </div>
  
          <script>
  
              let flashOn = false;
  
              function sendCommand(action) {
                  var xhr = new XMLHttpRequest();
                  xhr.open("GET", "/action?do=" + action, true);
                  xhr.send();
              }
  
              function toggleFlash() {
  
                  flashOn = !flashOn;
  
                  const switchElement = document.getElementById("flashSwitch");
                  const flashText = document.getElementById("flashText");
  
                  if (flashOn) {
                      sendCommand("flash-on");
                      flashText.textContent = "Flash: Encendido";
                  } else {
                      sendCommand("flash-off");
                      flashText.textContent = "Flash: Apagado";
                  }
  
              }
  
              function updateAngle(value, servoID) {
  
                  const slider = document.getElementById(servoID);
  
                  if (servoID == 'angleValue1') {
                      sendCommand("servo1-" + value);
                      slider.innerText = "Servo 1: " + value + "°";
                  } else if (servoID == 'angleValue2') {
                      sendCommand("servo2-" + value);
                      slider.innerText = "Servo 2: " + value + "°";
                  }
  
              }
  
              window.onload = function () {
  
                  document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
  
                  const switchElement = document.getElementById("flashSwitch");
                  switchElement.checked = false;
                  sendCommand("flash-off");
  
                  const slider1 = document.getElementById('angleSlider1');
                  slider1.value = 90;
                  sendCommand("servo1-90");
  
                  const slider2 = document.getElementById('angleSlider2');
                  slider2.value = 0;
                  sendCommand("servo2-0");
  
              };
  
          </script>
  
      </body>
  
  </html>     

)rawliteral";

// Función manejadora para solicitudes HTTP que devuelven la página de inicio
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html"); // Establece el tipo de contenido como HTML
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML)); // Envía la página HTML al cliente
}

// Función manejadora para el streaming de video
static esp_err_t stream_handler(httpd_req_t *req) {
  
  camera_fb_t *fb = NULL;             // Estructura para almacenar los fotogramas capturados
  esp_err_t res = ESP_OK;             // Variable para manejar errores
  size_t _jpg_buf_len = 0;            // Longitud del buffer de la imagen JPEG
  uint8_t *_jpg_buf = NULL;           // Puntero al buffer de la imagen JPEG
  char *part_buf[64];                 // Buffer para construir los encabezados de las partes del streaming

  // Establece el tipo de contenido como streaming de imágenes
  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  
  if (res != ESP_OK) {
    return res; // Retorna el error si no se pudo establecer el tipo de contenido
  }

  // Bucle infinito para enviar continuamente los fotogramas
  while (true) {
    // Captura un fotograma de la cámara
    fb = esp_camera_fb_get();

    if (!fb) { // Verifica si no se pudo capturar un fotograma
      res = ESP_FAIL; // Marca un error
    } else {
      // Solo procesa imágenes con un ancho mayor a 200
      if (fb->width > 200) {
        // Si el formato no es JPEG, convierte el fotograma a JPEG
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len); // Convierte a JPEG con calidad del 80%
          esp_camera_fb_return(fb); // Libera el buffer de la cámara
          fb = NULL;

          if (!jpeg_converted) { // Verifica si la conversión falló
            res = ESP_FAIL; // Marca un error
          }
        } else {
          // Si el formato ya es JPEG, obtiene directamente el buffer y su tamaño
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }

    // Envía los datos del fotograma si no hubo errores
    if (res == ESP_OK) {
      // Construye el encabezado para esta parte del streaming
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen); // Envía el encabezado del fotograma
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len); // Envía el contenido del fotograma
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY)); // Envía el delimitador entre fotogramas
    }

    // Libera recursos de la cámara o el buffer según corresponda
    if (fb) {
      esp_camera_fb_return(fb); // Libera el buffer de la cámara
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf); // Libera el buffer de la imagen JPEG si fue creado manualmente
      _jpg_buf = NULL;
    }

    if (res != ESP_OK) { // Sale del bucle si ocurre un error
      break;
    }
  }

  return res; // Retorna el resultado de la operación
  
}

// Función manejadora para comandos enviados al servidor HTTP
static esp_err_t cmd_handler(httpd_req_t *req) {
  
  char* buf;                     // Buffer para almacenar la consulta URL
  size_t buf_len;                // Longitud del buffer
  char variable[32] = {0,};      // Variable para almacenar el valor del parámetro "do"

  // Obtiene la longitud de la consulta URL
  buf_len = httpd_req_get_url_query_len(req) + 1;
  
  if (buf_len > 1) { // Verifica si hay una consulta URL presente
    buf = (char*)malloc(buf_len); // Reserva memoria para el buffer

    if (!buf) { // Verifica si falló la reserva de memoria
      httpd_resp_send_500(req);  // Envía un error 500 (interno del servidor)
      return ESP_FAIL;
    }

    // Obtiene la cadena de consulta URL
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      // Busca el valor del parámetro "do" en la consulta URL
      if (httpd_query_key_value(buf, "do", variable, sizeof(variable)) == ESP_OK) {
        // El parámetro "do" se encontró y su valor se almacena en `variable`
      } else {
        free(buf); // Libera la memoria del buffer
        httpd_resp_send_404(req); // Envía un error 404 (no encontrado)
        return ESP_FAIL;
      }
    } else {
      free(buf); // Libera la memoria del buffer
      httpd_resp_send_404(req); // Envía un error 404 (no encontrado)
      return ESP_FAIL;
    }
    
    free(buf); // Libera la memoria del buffer
  } else {
    httpd_resp_send_404(req); // Envía un error 404 (no encontrado)
    return ESP_FAIL;
  }

  // Ejecuta acciones según el valor del parámetro "do"
  if (!strcmp(variable, "flash-on")) { // Si el valor es "flash-on"
    digitalWrite(FLASH_LED_PIN, HIGH); // Enciende el LED del flash
  } else if (!strcmp(variable, "flash-off")) { // Si el valor es "flash-off"
    digitalWrite(FLASH_LED_PIN, LOW); // Apaga el LED del flash
  } else if (String(variable).startsWith("servo1-")) { // Si comienza con "servo1-"
    Serial.println("servo1-" + String(variable).substring(7)); // Muestra el valor en el monitor serial
  } else if (String(variable).startsWith("servo2-")) { // Si comienza con "servo2-"
    Serial.println("servo2-" + String(variable).substring(7)); // Muestra el valor en el monitor serial
  } else {
    return httpd_resp_send_500(req); // Envía un error 500 si el comando no es reconocido
  }

  // Agrega el encabezado de respuesta HTTP para permitir solicitudes CORS
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0); // Envía una respuesta vacía con código 200 (éxito)
  
}

// Función para iniciar el servidor de la cámara
void startCameraServer() {
  
  httpd_config_t config = HTTPD_DEFAULT_CONFIG(); // Configuración predeterminada del servidor HTTP
  config.server_port = 80; // Establece el puerto del servidor principal

  // Configura el manejador para la página de inicio
  httpd_uri_t index_uri = {
    .uri       = "/",          // URI para acceder a la página de inicio
    .method    = HTTP_GET,     // Método HTTP (GET)
    .handler   = index_handler,// Función manejadora para la página de inicio
    .user_ctx  = NULL          // Contexto de usuario (no se usa aquí)
  };

  // Configura el manejador para comandos
  httpd_uri_t cmd_uri = {
    .uri       = "/action",    // URI para los comandos
    .method    = HTTP_GET,     // Método HTTP (GET)
    .handler   = cmd_handler,  // Función manejadora para los comandos
    .user_ctx  = NULL          // Contexto de usuario (no se usa aquí)
  };

  // Configura el manejador para el streaming
  httpd_uri_t stream_uri = {
    .uri       = "/stream",    // URI para el streaming de video
    .method    = HTTP_GET,     // Método HTTP (GET)
    .handler   = stream_handler, // Función manejadora para el streaming
    .user_ctx  = NULL          // Contexto de usuario (no se usa aquí)
  };

  // Inicia el servidor HTTP principal
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri); // Registra el manejador para la página de inicio
    httpd_register_uri_handler(camera_httpd, &cmd_uri);   // Registra el manejador para los comandos
  }

  // Cambia los puertos para el servidor de streaming
  config.server_port += 1; // Incrementa el puerto del servidor
  config.ctrl_port += 1;   // Incrementa el puerto de control

  // Inicia el servidor HTTP para streaming
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri); // Registra el manejador para el streaming
  }
  
}

// Función de configuración inicial del programa
void setup() {

  // Desactiva la protección contra "brownout" para evitar reinicios inesperados por bajo voltaje
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Inicia la comunicación serial a 115200 baudios para depuración
  Serial.begin(115200);

  // Configura el pin del LED del flash como salida
  pinMode(FLASH_LED_PIN, OUTPUT);

  // Configuración de la cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;       // Canal LEDC para señal de reloj de la cámara
  config.ledc_timer = LEDC_TIMER_0;           // Temporizador LEDC
  
  // Pines para los datos de la cámara (D0 - D7)
  config.pin_d0 = Y2_GPIO_NUM;                // Pin D0 para datos
  config.pin_d1 = Y3_GPIO_NUM;                // Pin D1 para datos
  config.pin_d2 = Y4_GPIO_NUM;                // Pin D2 para datos
  config.pin_d3 = Y5_GPIO_NUM;                // Pin D3 para datos
  config.pin_d4 = Y6_GPIO_NUM;                // Pin D4 para datos
  config.pin_d5 = Y7_GPIO_NUM;                // Pin D5 para datos
  config.pin_d6 = Y8_GPIO_NUM;                // Pin D6 para datos
  config.pin_d7 = Y9_GPIO_NUM;                // Pin D7 para datos
  
  // Pines de sincronización y control
  config.pin_xclk = XCLK_GPIO_NUM;            // Pin para el reloj externo de la cámara (XCLK)
  config.pin_pclk = PCLK_GPIO_NUM;            // Pin para la señal de reloj de píxeles (PCLK)
  config.pin_vsync = VSYNC_GPIO_NUM;          // Pin para la sincronización vertical (VSYNC)
  config.pin_href = HREF_GPIO_NUM;            // Pin para la referencia horizontal (HREF)
  
  // Pines de comunicación SCCB (interfaz de control de la cámara)
  config.pin_sscb_sda = SIOD_GPIO_NUM;        // Pin para datos SCCB (SIOD)
  config.pin_sscb_scl = SIOC_GPIO_NUM;        // Pin para reloj SCCB (SIOC)
  
  // Pines para control de la cámara
  config.pin_pwdn = PWDN_GPIO_NUM;            // Pin para el modo de ahorro de energía (opcional)
  config.pin_reset = RESET_GPIO_NUM;          // Pin para reiniciar la cámara (RESET)

  // Configuración adicional de la cámara
  config.xclk_freq_hz = 15000000;             // Frecuencia del reloj externo en Hz (15 MHz)
  config.pixel_format = PIXFORMAT_JPEG;       // Formato de píxeles (JPEG para ahorro de espacio)
  config.frame_size = FRAMESIZE_VGA;          // Tamaño del cuadro (640x480 píxeles)
  config.jpeg_quality = 20;                   // Calidad del JPEG (más bajo, mejor calidad)
  config.fb_count = 2;                        // Número de búferes de cuadro (doble búfer)

  // Inicializa la cámara con la configuración especificada
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) { // Verifica si la inicialización falló
    return; // Sal del setup si no se pudo inicializar la cámara
  }

  // Conecta el dispositivo a la red WiFi
  WiFi.begin(ssid, password); // Inicia la conexión WiFi usando las credenciales
  while (WiFi.status() != WL_CONNECTED) { // Espera hasta que se establezca la conexión
    delay(500); // Espera 500 ms antes de verificar nuevamente
  }

  // Inicia el servidor HTTP de la cámara
  startCameraServer();
  
}

// Bucle principal del programa (se ejecuta repetidamente)
void loop() {
  // No se realiza ninguna operación en el bucle principal
}