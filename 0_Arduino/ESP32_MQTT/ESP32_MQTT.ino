#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "config.h"

// Replace the next variables with your SSID/Password combination
//const char* ssid = "yellowSub";
//const char* password = "Protools1234";
//const char* mqtt_server = "10.0.0.45";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


// Define Pin
const int networkLED = 2;
const int inValve = 16;
const int outValve = 17;
const int airPressureSensor = 18;

void setup() {
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();

  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //Network Statue Indicator
  pinMode(networkLED, OUTPUT);
  //Valves Pins
  pinMode(inValve, OUTPUT);
  pinMode(outValve, OUTPUT);
  //Air Pressure Sensor
  pinMode(airPressureSensor, INPUT);
}

void connectWiFi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(networkLED, HIGH);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  //if (String(topic) == "test/esp32/output") {
  if (String(topic) == "test/esp32/inValve") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(inValve, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(inValve, LOW);
    }
  }
}

void connectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(device_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("test/+/inValve");
      client.subscribe("test/+/outValve");
      //LED lit up
      digitalWrite(networkLED, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      digitalWrite(networkLED, LOW);
      delay(1000);
      digitalWrite(networkLED, HIGH);
      delay(1000);
    }
  }
}


void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
  analogRead();
  
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    client.publish("test/esp32/airPressure", "100");
    //Short blink when sending 
    digitalWrite(networkLED, LOW);
    delay(50);
    digitalWrite(networkLED, HIGH);
    delay(50);
  }
}
