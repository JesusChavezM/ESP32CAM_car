
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>


#define CAMERA_MODEL_AI_THINKER

const char* ssid = "iPhone de Alexis";  //Enter SSID WIFI Name
const char* password = "A1234567";               //Enter WIFI Password

#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22


#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#else
#error "Camera model not selected"
#endif

WebServer server(80);


const int gpLb = 2;   // Left Backward
const int gpLf = 14;  // Left Forward
const int gpRb = 15;  // Right Backward
const int gpRf = 13;  // Right Forward
const int gpLed = 4;  // Light
extern String WiFiAddr = "";

void setupCamera();
void handleRoot();
void handleCapture();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  setupCamera();

  pinMode(gpLb, OUTPUT);   //Left Backward
  pinMode(gpLf, OUTPUT);   //Left Forward
  pinMode(gpRb, OUTPUT);   //Right Forward
  pinMode(gpRf, OUTPUT);   //Right Backward
  pinMode(gpLed, OUTPUT);  //Light

  //initialize
  digitalWrite(gpLb, LOW);
  digitalWrite(gpLf, LOW);
  digitalWrite(gpRb, LOW);
  digitalWrite(gpRf, LOW);
  digitalWrite(gpLed, LOW);



  // Conecta a la red WiFi
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");

  Serial.print("ESP32-CAM Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' para conectar");

  // Configura las rutas del servidor web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/cam-hi.jpg", HTTP_GET, handleCapture);

  server.on("/forward", HTTP_GET, []() {
    handleCommand("forward");
  });
  server.on("/backward", HTTP_GET, []() {
    handleCommand("backward");
  });
  server.on("/left", HTTP_GET, []() {
    handleCommand("left");
  });
  server.on("/right", HTTP_GET, []() {
    handleCommand("right");
  });
  server.on("/stop", HTTP_GET, []() {
    handleCommand("stop");
  });
  server.on("/led_on", HTTP_GET, []() {
    handleCommand("led_on");
  });
  server.on("/led_off", HTTP_GET, []() {
    handleCommand("led_off");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}

void handleCommand(String command) {
  if (command == "forward") {
    digitalWrite(gpLf, HIGH);
    digitalWrite(gpRf, HIGH);
    digitalWrite(gpLb, LOW);
    digitalWrite(gpRb, LOW);
  } else if (command == "backward") {
    digitalWrite(gpLf, LOW);
    digitalWrite(gpRf, LOW);
    digitalWrite(gpLb, HIGH);
    digitalWrite(gpRb, HIGH);
  } else if (command == "left") {
    digitalWrite(gpLf, LOW);  // Llanta izquierda delantera parada
    digitalWrite(gpRf, HIGH); // Llanta derecha delantera avanza
    digitalWrite(gpLb, LOW);  // Llanta izquierda trasera parada
    digitalWrite(gpRb, HIGH); // Llanta derecha trasera avanza
  } else if (command == "right") {
    digitalWrite(gpLf, HIGH);  // Llanta izquierda delantera avanza
    digitalWrite(gpRf, LOW);   // Llanta derecha delantera parada
    digitalWrite(gpLb, HIGH);  // Llanta izquierda trasera avanza
    digitalWrite(gpRb, LOW);
  } else if (command == "stop") {
    digitalWrite(gpLf, LOW);
    digitalWrite(gpRf, LOW);
    digitalWrite(gpLb, LOW);
    digitalWrite(gpRb, LOW);
  } else if (command == "led_on") {
    digitalWrite(gpLed, HIGH);
  } else if (command == "led_off") {
    digitalWrite(gpLed, LOW);
  }
  server.send(200, "text/plain", "OK");
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t* s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);
}

void handleRoot() {
  String html = "<html><body><h1>ESP32-CAM</h1><img src=\"/cam-hi.jpg\" style='width:300px;height:300px; transform:rotate(90deg);'/></body></html>";
  server.send(200, "text/html", html);
}

void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}