#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

class ShutterRelay {
  public:
    ShutterRelay(uint8_t lowPin, uint8_t highPin, long operationDuration, PubSubClient *client, String topic) : lowPin_(lowPin), highPin_(highPin), operationDuration_(operationDuration), 
    client_(client), topic_(topic) { 
      pinMode(lowPin_, OUTPUT);
      pinMode(highPin_, OUTPUT);
    }

    /**
      control shutter relay based on message, first deactivate(reset) pins to avoid damaging the relay 
    */
    void control(String message) {
      Serial.println("Shutter control requested: go " + message);
      resetState();
      if(message == "UP") {
        activate();
        digitalWrite(highPin_, LOW);
        setStatus("GOING_UP");
        lastOperationUp_ = true;
      } else if(message == "DOWN") {
        activate();
        digitalWrite(lowPin_, LOW);
        setStatus("GOING_DOWN");
        lastOperationUp_ = false;
      } else {
        setStatus("MIDDLE");
        deactivate();
      }
    }

    /**
      reset the pins to avoid damaging the relay 
    */
    void resetState() {
      digitalWrite(highPin_, HIGH);
      digitalWrite(lowPin_, HIGH);
    }

    /**
      subscribe to MQTT topic
    */
    void connect() {
      char topic[20];
      snprintf(topic, 20, "/%d", topic_);
      client_->subscribe(topic);
    }

    /**
      publish shutter status to MQTT topic
    */
    void setStatus(String status) {
      char topic[20];
      snprintf(topic, 20, "/%d/status", topic_);
      char payload[20];
      snprintf(payload, 20, "%s", status.c_str());
      client_->publish(topic, payload);
    }

    /**
      mark the shutter as active, performing operaion
    */
    void activate() {
      active_ = true;
      lastOperationStartTime_ = millis();
    }

    /**
      mark the shutter as inactive and reset the pins
    */
    void deactivate() {
      active_ = false;
      resetState();
    }

    bool isActive() {
      return active_;
    }

    /**
      check if the shutter is still performing its last operation (operation duration may vary based on the shutter's size). 
      if the max operation duration is exceeded the shutter has finished its operationfor sure, so deactivate it
      and set the status as finished based on the last opeartion
    */
    void checkActive() {
      if(active_ && (millis() - lastOperationStartTime_ > operationDuration_)) {
        deactivate();
        setStatus(lastOperationUp_ ? "UP" : "DOWN");
      }
    }

  private:
    uint8_t lowPin_;
    uint8_t highPin_;
    long operationDuration_;
    PubSubClient *client_;
    String topic_;
    bool active_;
    long lastOperationStartTime_;
    bool lastOperationUp_;
};

unsigned long currentMillis;

WiFiClient espClient;
PubSubClient client(espClient);

// Replace with your network credentials
const char* ssid = "YourSSID";
const char* password = "YourPassword";

// Replace with the MQTT broker IP and port
const char* mqtt_server = "broker.example.com";
const int mqtt_port = 1883;


// DHT sensor setup
const long DHT_READ_FREQUENCY = 60000;
#define DHTTYPE DHT22
DHT dhtD1(2, DHTTYPE);
DHT dhtD2(3, DHTTYPE);


// Pin numbers for flood sensors
const int floodSensorPin = 13;


const long SHUTTER_LONG_OPERATION_DURATION = 20000;
const long SHUTTER_SHORT_OPERATION_DURATION = 15000;
const long SHUTTER_ACTIVE_CHECK_FREQUENCY = 60000;
ShutterRelay shutterM10(7, 8, SHUTTER_LONG_OPERATION_DURATION, &client, "M10");
ShutterRelay shutterM11(9, 10, SHUTTER_LONG_OPERATION_DURATION, &client, "M11");
ShutterRelay shutterM12(11, 12, SHUTTER_SHORT_OPERATION_DURATION, &client, "M12");
ShutterRelay shutterM13(13, 14, SHUTTER_SHORT_OPERATION_DURATION, &client, "M13");


/**
  read all DHT sensors every DHT_READ_FREQUENCY miliseconds
*/
void readDHTValues() {
  static unsigned long lastReadMillis = 0;
  if (currentMillis - lastReadMillis > DHT_READ_FREQUENCY) {
    readDHT(dhtD1, "D1");
    readDHT(dhtD2, "D2");
  }
}

/**
  read DHT sensor humidity and temperature values and publish to topic
*/
void readDHT(DHT dht, String topic) {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor");
    return;
  }
  char topicTemperature[20];
  snprintf(topicTemperature, 20, "/%d/temperature", topic);
  char payloadTemperature[20];
  snprintf(payloadTemperature, 20, "%.2f", temperature);
  client.publish(topicTemperature, payloadTemperature);

  char topicHumidity[20];
  snprintf(topicHumidity, 20, "/%d/humidity", topic);
  char payloadHumidity[20];
  snprintf(payloadHumidity, 20, "%.2f", humidity);
  client.publish(topicHumidity, payloadHumidity);

  Serial.println("Published temperature and humidity for DHT sensor: " + topic);
}

/**
  every SHUTTER_ACTIVE_CHECK_FREQUENCY milliseconds check if any shutter is still marked as active but finished its operation 
*/
void checkShuttersStillActive() {
  static unsigned long lastActiveMillis = 0;
  if (currentMillis - lastActiveMillis > SHUTTER_ACTIVE_CHECK_FREQUENCY) {
    shutterM10.checkActive();
    shutterM11.checkActive();
    shutterM12.checkActive();
    shutterM13.checkActive();
  }
}

void readFloodSensorValues() {

 // Read the state of the flood sensor
    int floodSensorState = digitalRead(floodSensorPin);

    // Publish a message if a leak is detected
    if (floodSensorState == LOW) {
      client.publish("leakDetected", "1");
    }
}

/**
  MQTT messages comming in handler 
*/
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived in topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(message);

  if(topic == "M10") {
    shutterM10.control(message);
  } else if(topic == "M11") {
    shutterM11.control(message);
  } else if(topic == "M12") {
    shutterM12.control(message);
  } else if(topic == "M13") {
    shutterM13.control(message);
  } else {
    Serial.println("Nothing to do here.");
  }
}

void setup() {
  Serial.begin(115200);

  dhtD1.begin();
  dhtD2.begin();

  // Set the pin mode for the flood sensor
  pinMode(floodSensorPin, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ArduinoClient")) {
      Serial.println("Connected to MQTT broker");

    } else {
      Serial.println("Connection to MQTT broker failed, retrying...");
      delay(5000);
    }
  }

  shutterM10.connect();
  shutterM11.connect();
  shutterM12.connect();
  shutterM13.connect();
}

void loop() {
  currentMillis = millis();
  // Keep the connection to the MQTT broker alive
  client.loop();

  readDHTValues();
  readFloodSensorValues();
  checkShuttersStillActive();
  
}
