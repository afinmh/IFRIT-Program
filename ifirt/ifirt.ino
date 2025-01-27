#include <WiFi.h>
#include <WebSocketsClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <HTTPClient.h>

// Konfigurasi Wi-Fi
const char* ssid = "Wong Ayu";        // Ganti dengan SSID Wi-Fi kamu
const char* password = "4sehat5sempurna"; // Ganti dengan password Wi-Fi kamu

// Konfigurasi MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic_sensor = "motorffitenass/sensor";

WiFiClient espClient;
PubSubClient client(espClient);
WebSocketsClient webSocket;

// Fungsi callback untuk MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Fungsi untuk menangani event WebSocket
void onEvent(WStype_t type, uint8_t * payload, size_t length) {
  String message;

  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Connected to WebSocket server");
      // Kirim pesan untuk mendaftar sebagai receiver
      message = "{\"type\": \"receiver\"}";
      webSocket.sendTXT(message);
      Serial.println("Sent: Receiver registered");
      break;

    case WStype_DISCONNECTED:
      Serial.println("Disconnected from WebSocket server");
      break;

    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.println("Received binary message");
      break;

    default:
      break;
  }
}

// Fungsi untuk menghubungkan ke MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Membuat ID unik untuk client
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Hubungkan ke broker
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Task untuk menangani WebSocket
void taskWebSocket(void* pvParameters) {
  for (;;) {
    webSocket.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay kecil untuk multitasking
  }
}

// Task untuk menangani MQTT dan publish data setiap 1 detik
void taskMQTT(void* pvParameters) {
  for (;;) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    // Simulasi data random untuk publish ke MQTT
    String pompa = random(0, 2) ? "ON" : "OFF";
    String strobo = random(0, 2) ? "ON" : "OFF";
    String speaker = random(0, 2) ? "ON" : "OFF";
    String fire = random(0, 2) ? "Bahaya" : "Aman";
    String batre = String(random(10, 100));
    float distance = random(1, 100) / 10.0;

    // Format data sebagai JSON
    String jsonData = "{\"pompa\":\"" + pompa + "\",\"strobo\":\"" + strobo + "\",\"speaker\":\"" + speaker + "\",\"fire\":\"" + fire + "\",\"batre\":\"" + batre + "\",\"distance\":" + String(distance) + ",\"Car\":\"Connect\"}";

    // Publish data ke topik MQTT
    client.publish(topic_sensor, jsonData.c_str());
    Serial.println("Data published: " + jsonData);

    // Delay untuk simulasi interval
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  // Inisialisasi serial monitor
  Serial.begin(115200);

  // Koneksi ke Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Inisialisasi MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Inisialisasi WebSocket
  webSocket.beginSSL("dusty-steel-atlasaurus.glitch.me", 443, "/");
  webSocket.onEvent(onEvent);

  // Buat task untuk WebSocket dan MQTT
  xTaskCreate(taskWebSocket, "WebSocket Task", 4096, NULL, 1, NULL);
  xTaskCreate(taskMQTT, "MQTT Task", 4096, NULL, 1, NULL);
}

void loop() {
  // Loop utama kosong karena tugas ditangani oleh RTOS tasks
}
