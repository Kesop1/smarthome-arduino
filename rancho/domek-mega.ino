#include <ArduinoSTL.h>  // Workaround: downgrade Arduino AVR Boards to 1.8.2. https://github.com/mike-matera/ArduinoSTL/issues/56
#include <string.h>
#include <map>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

#define RELAY_DEFAULT_OFF HIGH
#define M10_HIGH  22
#define M10_LOW   23
#define M11_HIGH  24
#define M11_LOW   25
#define M12_HIGH  26
#define M12_LOW   27
#define M13_HIGH  28
#define M13_LOW   29


std::map<std::string, int> shutterRelaysHighMap;
std::map<std::string, int> shutterRelaysLowMap;

unsigned long currentMillis;
unsigned long lastShutterRelayCommandMillis;
const long MAX_SHUTTER_RELAY_COMMAND_PROCESSING_TIME = 20000;

// Replace with your network details
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress ETH_IP_ADDRESS(192, 168, 1, 103);
IPAddress ETH_GATEWAY(192, 168, 1, 1);
IPAddress ETH_SUBNET(255, 255, 255, 0);

EthernetClient ethClient;
PubSubClient client(ethClient);

// Replace with the MQTT broker IP and port
const char* MQTT_SERVER_ADDRESS = "192.168.1.214";
const int MQTT_SERVER_PORT = 1883;
const char* DEVICE_NAME = "arduinoMega";
const char* DEVICE_AVAILABILITY_TOPIC = (String(DEVICE_NAME) + "/available").c_str();
const char* HOME_ASSISTANT_USER = "homeassistant";
const char* HOME_ASSISTANT_PASSWORD = "";

std::map<std::string, int> sensorPinsMap;

// DHT sensor setup
const long TEMPERATURE_READ_FREQUENCY = 600000;
#define DHTTYPE DHT11  //TODO
DHT dhtD9(2, DHTTYPE);
DHT dhtD2(3, DHTTYPE);

//floor sensors setup
const long FLOOR_TEMPERATURE_READ_FREQUENCY = 60000;
#define D1_FLOOR_SENSOR_PIN A0

typedef enum {
  SHUTTER_RELAY,
  RELAY,
  BME280_SENSOR,
  NTC_TEMPERATURE_SENSOR_10K
} objectType;

std::map<std::string, objectType> objectsMap;



// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.print("Starting device ");
  Serial.println(DEVICE_NAME);
  initializeShutterRelays();
  initializeDhtSensors();
  initializeFlorTemperatureSensors();

  // Connect to Ethernet
  Ethernet.begin(mac, ETH_IP_ADDRESS, ETH_GATEWAY, ETH_SUBNET);

  // Connect to MQTT broker
  client.setServer(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void initializeShutterRelays() {
  objectsMap["m10"] =             SHUTTER_RELAY;
  shutterRelaysHighMap["m10"] =   M10_HIGH;
  shutterRelaysLowMap["m10"] =    M10_LOW;
  objectsMap["m11"] =             SHUTTER_RELAY;
  shutterRelaysHighMap["m11"] =   M11_HIGH;
  shutterRelaysLowMap["m11"] =    M11_LOW;
  objectsMap["m12"] =             SHUTTER_RELAY;
  shutterRelaysHighMap["m12"] =   M12_HIGH;
  shutterRelaysLowMap["m12"] =    M12_LOW;
  objectsMap["m13"] =             SHUTTER_RELAY;
  shutterRelaysHighMap["m13"] =   M13_HIGH;
  shutterRelaysLowMap["m13"] =    M13_LOW;
  // Iterate through the  map and set pinMode to OUTPUT for each pin
  for (auto const& [deviceName, pinNumber] : shutterRelaysHighMap) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, RELAY_DEFAULT_OFF);
  }
  for (auto const& [deviceName, pinNumber] : shutterRelaysLowMap) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, RELAY_DEFAULT_OFF);
  }
}

void initializeFlorTemperatureSensors() {
  objectsMap["d1"] = NTC_TEMPERATURE_SENSOR_10K;
  sensorPinsMap["d1"] = D1_FLOOR_SENSOR_PIN;
}

void initializeDhtSensors() {
  dhtD9.begin();
  dhtD2.begin();
}

/**
  MQTT messages comming in handler 
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived in topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(message);
  if (isShutterRelay(topic)) {
    moveShutter(topic, message);
  } else if (isRelay(topic)) {
    Serial.println("relay");
  }
}

boolean isShutterRelay(char* deviceName) {
  return objectsMap[deviceName] == SHUTTER_RELAY;
}

boolean isRelay(char* deviceName) {
  return objectsMap[deviceName] == RELAY;
}

void moveShutter(char* shutterRelay, String command) {
  Serial.println("Move shutter " + String(shutterRelay) + " command " + command + " received");
  lastShutterRelayCommandMillis = currentMillis;
  int pinHigh = shutterRelaysHighMap[shutterRelay];
  int pinLow = shutterRelaysLowMap[shutterRelay];
  digitalWrite(pinHigh, RELAY_DEFAULT_OFF);
  digitalWrite(pinLow, RELAY_DEFAULT_OFF);
  if (command == "UP") {
    publishDeviceStatus(shutterRelay, "GOING UP");
    delay(20);
    digitalWrite(pinHigh, !RELAY_DEFAULT_OFF);
  } else if (command == "DOWN") {
    publishDeviceStatus(shutterRelay, "GOING DOWN");
    delay(20);
    digitalWrite(pinLow, !RELAY_DEFAULT_OFF);
  } else if (command == "STOP") {
    publishDeviceStatus(shutterRelay, "UNKNOWN");
  } else if (command == "RESET") {
    publishDeviceStatus(shutterRelay, "RESET");
  } else {
    Serial.println("Unrecognized shutter relay command " + command + "!");
  }
}

// ---------------------------------------------------------------------------

void loop() {
  currentMillis = millis();
  // Keep the connection to the MQTT broker alive
  if (!client.connected()) {
    reconnectToMQTT();
  }
  client.loop();

  readDHTValues();
  readFloorSensors();
  resetShutterRelays();
}

void reconnectToMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String availability = String(DEVICE_NAME) + "/available";
    int topicLength = availability.length() + 1;
    char availabilityTopic[topicLength];
    availability.toCharArray(availabilityTopic, topicLength);
    if (client.connect(DEVICE_NAME, HOME_ASSISTANT_USER, HOME_ASSISTANT_PASSWORD, availabilityTopic, 0, true, "offline")) {
      Serial.println("connected");
      client.publish(availabilityTopic, "online", true);
      subscribeToTopics();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
  read all DHT sensors every TEMPERATURE_READ_FREQUENCY miliseconds
*/
void readDHTValues() {
  static unsigned long lastReadMillis = 0;
  if (currentMillis - lastReadMillis > TEMPERATURE_READ_FREQUENCY) {
    lastReadMillis = currentMillis;
    readDHT(dhtD9, "d9");
    readDHT(dhtD2, "d2");
  }
}

/**
  read all NTC 10k Ohm temperature sensors every FLOOR_TEMPERATURE_READ_FREQUENCY miliseconds
*/
void readFloorSensors() {
  static unsigned long lastReadMillis = 0;
  if (currentMillis - lastReadMillis > FLOOR_TEMPERATURE_READ_FREQUENCY) {
    lastReadMillis = currentMillis;
    for (auto const& [deviceName, objectType] : objectsMap) {
      if (objectType != NTC_TEMPERATURE_SENSOR_10K) {
        continue;
      }
      char* deviceChar = deviceName.c_str();
      String device = deviceChar;
      
      int Vo;
      float R1 = 10000;
      float logR2, R2, T, Tc;
      float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
      int samples = 5;
      float value = 0;
      for(int x = 0; x < samples; x++) {
        Vo = analogRead(sensorPinsMap[deviceChar]);
        R2 = R1 * (1023.0 / (float)Vo - 1.0);
        logR2 = log(R2);
        T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
        Tc = T - 273.15;
        value += Tc;
        delay(500);//TODO DELAY!!!      
      }
      value = value / samples;
      
      Serial.print(device);
      Serial.print(" floor sensor Temperature: ");
      Serial.print(value);
      Serial.println(" C");

      publishSensorValue(device, String(value));
    }
  }
}

/*
  publish sensor value to device topic
*/
void publishSensorValue(String device, String value) {
  int deviceLength = device.length() + 1;
  char topic[deviceLength];
  device.toCharArray(topic, deviceLength);
  
  int valueLength = value.length() + 1;
  char payload[valueLength];
  value.toCharArray(payload, valueLength);
  client.publish(topic, payload);
}

/*
  publish device status to device status topic
*/
void publishDeviceStatus(String device, String value) {
  String status = device + "/state";
  int topicLength = status.length() + 1;
  char statusTopic[topicLength];
  status.toCharArray(statusTopic, topicLength);
  
  int valueLength = value.length() + 1;
  char payload[valueLength];
  value.toCharArray(payload, valueLength);
  client.publish(statusTopic, payload);
}

/**
  read DHT sensor humidity and temperature values and publish to topic
*/
void readDHT(DHT dht, String topic) {
  Serial.println(topic);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.println(F("°C"));
  Serial.print(", humidity: ");
  Serial.println(humidity);
  Serial.println(F("%"));
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor");
    return;
  }
  char topicTemperature[20];//TODO publishSensorValue
  String tempTopic = topic + "/temperature";
  tempTopic.toCharArray(topicTemperature, 20);
  char payloadTemperature[20];
  String temperatureValue = String(temperature);
  temperatureValue.toCharArray(payloadTemperature, 20);
  client.publish(topicTemperature, payloadTemperature);

  char topicHumidity[20];
  String humTopic = topic + "/humidity";
  humTopic.toCharArray(topicHumidity, 20);
  char payloadHumidity[20];
  String humidityValue = String(humidity);
  humidityValue.toCharArray(payloadHumidity, 20);
  client.publish(topicHumidity, payloadHumidity);

  Serial.println("Published temperature and humidity for DHT sensor: " + topic);
}

void subscribeToTopics() {
  subscribeShutterRelays();
  subscribeRelays();
}

void subscribeShutterRelays() {
  Serial.println("Subscribing shutter relays");
  for (auto const& [deviceName, objectType] : objectsMap) {
    if (objectType != SHUTTER_RELAY) {
        continue;
      }
    client.subscribe(deviceName.c_str());
  }
}

void subscribeRelays() {
  Serial.println("Subscribing relays");
  for (auto const& [deviceName, objectType] : objectsMap) {
    if (objectType != RELAY) {
      continue;
    }
    client.subscribe(deviceName.c_str());
  }
}

/**
  reset shutter relays pins MAX_SHUTTER_RELAY_COMMAND_PROCESSING_TIME aftre the lst received command to avoid setting both pins to HIGH, which could cause shutter damage
*/
void resetShutterRelays() {
  if (lastShutterRelayCommandMillis != 0 && currentMillis - lastShutterRelayCommandMillis > MAX_SHUTTER_RELAY_COMMAND_PROCESSING_TIME) {
    Serial.println("Resetting relays pins");
    lastShutterRelayCommandMillis = 0;
    for (auto const& [deviceName, objectType] : objectsMap) {
      if (objectType != SHUTTER_RELAY) {
        continue;
      }
      char* deviceChar = deviceName.c_str();
      moveShutter(deviceChar, "RESET");
    }
  }
}
