#include <Arduino.h>
#include <AceButton.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

using namespace ace_button;

const char* ssid = "Liiit";
const char* password = "123456789";

WebSocketsServer webSocket = WebSocketsServer(80);

const int bUpPin = 21;
const int LED_PIN = 17;
int LED_VALUE = 0;

ButtonConfig bUpConfig;
AceButton bUp(&bUpConfig);

void bUpEvent(AceButton*, uint8_t, uint8_t);

void changeBrightness(int value) {
  Serial.println(value);
  if (value < 255 && value >= 0) {
    LED_VALUE = value;
    digitalWrite(LED_PIN, LED_VALUE);
    // TODO: Send BLE Message.
  }
}

void onWebSocketEvent(
  uint8_t num,
  WStype_t type,
  uint8_t * payload,
  size_t length
) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;
    case WStype_TEXT:
      {
        changeBrightness(atoi((const char *) payload));
        Serial.printf("[%u] Text: %s\n", num, payload);
        webSocket.sendTXT(num, payload);
      }      
      break;
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void setupWS() {
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void setupAP() {
  WiFi.softAP(ssid, password, 12, 1, 5);
  IPAddress IP = WiFi.softAPIP(); 
  Serial.print("AP IP address: "); 
  Serial.println(IP);
}

void setupLED() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_VALUE);
}

void setupButton() {
  pinMode(bUpPin, INPUT_PULLUP);
  bUpConfig.setEventHandler(bUpEvent);
  bUpConfig.setFeature(ButtonConfig::kFeatureClick);
  bUp.init(bUpPin, HIGH);
}

void setup() {
  Serial.begin(115200);
  setupAP();
  setupLED();
  setupButton();
  setupWS();
}

void loop() {
  bUp.check();
  webSocket.loop();
}

void bUpEvent(
  AceButton*, 
  uint8_t eventType,
  uint8_t buttonState
) {
  switch (eventType) {
    case AceButton::kEventPressed:
      if (LED_VALUE < 255) {
        changeBrightness(LED_VALUE += 51);
      } else {
        changeBrightness(0);
      }
      break;
  }
}
