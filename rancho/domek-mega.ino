#include <ArduinoSTL.h>  // Workaround: downgrade Arduino AVR Boards to 1.8.2. https://github.com/mike-matera/ArduinoSTL/issues/56
#include <string.h>
#include <map>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

#define RELAY_DEFAULT_OFF HIGH
#define M10_HIGH 21
#define M10_LOW 22

std::map<std::string, int> shutterRelaysHighMap;
std::map<std::string, int> shutterRelaysLowMap;

unsigned long currentMillis;

// Replace with your network details
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress ip(192, 168, 1, 103);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

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
  Ethernet.begin(mac, ip, gateway, subnet);

  // Connect to MQTT broker
  client.setServer(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void initializeShutterRelays() {
  objectsMap["m10"] = SHUTTER_RELAY;
  shutterRelaysHighMap["m10"] = M10_HIGH;
  shutterRelaysLowMap["m10"] = M10_LOW;

  objectsMap["M11"] = RELAY;  //TODO

  shutterRelaysHighMap["M11"] = 23;
  shutterRelaysLowMap["M11"] = 24;
  shutterRelaysHighMap["M12"] = 25;
  shutterRelaysLowMap["M12"] = 26;
  shutterRelaysHighMap["M13"] = 27;
  shutterRelaysLowMap["M13"] = 28;

  // Iterate through the  map and set pinMode to OUTPUT for each pin
  for (auto const& [deviceName, pinNumber] : shutterRelaysHighMap) {
    pinMode(pinNumber, OUTPUT);
  }
  for (auto const& [deviceName, pinNumber] : shutterRelaysLowMap) {
    pinMode(pinNumber, OUTPUT);
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
    Serial.println("moveShutter");
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

void moveShutter(char* shutterRelay, char* command) {
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
        delay(500);      
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
