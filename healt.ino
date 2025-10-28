//Gracias por usar 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- CONFIG ----------------
const char* ssid = "xxxxxx";
const char* password = "xxxx";
const char* serverURL = "URLFLASK:5000/health";
const int pollingInterval = 100; // ms
const int led1 = 2; // GPIO2
const int led2 = 4; // GPIO4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 14 // D6
#define OLED_SCL 12 // D5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiClient client;
int lastHealth = -1;
bool led4On = false;
unsigned long led4Timer = 0;
void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
// un check inicial
  for (int i = 0; i < 3; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    delay(200);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(200);
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Conectando...");
  display.display();

  // Conectar Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi OK");
  display.display();
  delay(1000);
}

void loop() {
  if (led4On && millis() - led4Timer > 1000) {
    digitalWrite(led2, LOW);
    led4On = false;
    Serial.println("LD4 apagado");
  }

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, serverURL);

    int httpCode = http.GET();

    if(httpCode == 200) {
      String payload = http.getString();
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if(!error) {
        int health = doc["health"];
        Serial.println("Vida: " + String(health));
        if (lastHealth != -1) {
          if (health < lastHealth) {
            digitalWrite(led1, HIGH);
            delay(200);
            digitalWrite(led1, LOW);
            digitalWrite(led2, HIGH);
            led4On = true;
            led4Timer = millis();
          } 
          else if (health > lastHealth && led4On) {
            digitalWrite(led2, LOW);
            led4On = false;
          }
        }
        if (health == 0) {
          digitalWrite(led2, HIGH);
          led4On = true;
          led4Timer = millis();
        }
        lastHealth = health;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("GTA V Health");
        display.setTextSize(3);
        display.setCursor(0, 20);
        display.print(health);
        display.print("%");
        display.display();
      } else {
        Serial.println("Error parsing JSON");
      }
    } else {
      Serial.println("Error HTTP: " + String(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }
  delay(pollingInterval);
}
