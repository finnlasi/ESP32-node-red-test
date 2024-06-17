#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP32Servo.h>
#include <PubSubClient.h>

#define LedPin 2 // ESP32 in-board LED pin
const int sensorPin = 25;
Servo myservo;
const char* ssid = "zaimi75_2.4GHz@unifi"; // WiFi SSID
const char* password = "ic750222"; // WiFi Password
const char* mqtt_server = "broker.hivemq.com"; // hivemq Server URL

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String string;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  for (int i = 0; i < length; i++) {
    string += (char)payload[i];
  }
  Serial.println(string);

  if (strcmp(topic, "/LedControl") == 0) {
    if (string == "a") {
      digitalWrite(LedPin, HIGH);
    } else if (string == "b") {
      digitalWrite(LedPin, LOW);
    }
  }
  
  if (strcmp(topic, "/ServoControl") == 0) {
    if (isNumeric(string)) {
      int status = string.toInt();
      int pos = map(status, 1, 100, 0, 180);
      Serial.println(pos);
      myservo.write(pos);
      delay(15);
    } else {
      Serial.println("Received non-numeric payload for servo control");
    }
  }
}

bool isNumeric(String str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    if (client.connect("ESPClient")) {
      Serial.println("Connected");
      client.subscribe("/LedControl");
      client.subscribe("/ServoControl");
      client.subscribe("/Vibration");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LedPin, OUTPUT);
  myservo.attach(4);
  pinMode(sensorPin, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

