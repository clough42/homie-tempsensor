/**
 * Dual DHT Temperature Sensor
 *
 * Uses Homie-ESP8266 framework to connect to the network and MQTT, read up
 * to two DHT sensors, report data and then deep sleep until the next reading.
 * Default configuration is for a refrigerator and a freezer, but can be
 * customized for other applications.
 *
 * Reports for each sensor:  Temperature (celsius), Temperature (fahrenheit),
 * Relative Humidity and Heat Index.  Also reports battery voltage, if you
 * provide an appropriate voltage divider to an analog input.
 */

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Homie.h>

// Homie node names - Comment out either to disable
#define NODE_1 "freezer"
#define NODE_2 "refrigerator"

#define DHT_PIN_1 D4
#define DHT_PIN_2 D3
#define DHT_TYPE DHT22
#define DEEP_SLEEP_SECONDS 300

// WeMos D1 Mini with extra 215K resistor
// for total 415K/100K voltage divider
// on ESP8266 A0
#define VOLTAGE_PIN A0
#define VOLTAGE_COEFFICIENT 0.0055

HomieNode batteryNode("battery", "voltage");

#ifdef NODE_1
HomieNode dhtNode1(NODE_1, "dht");
DHT dht1(DHT_PIN_1, DHT_TYPE);
bool temp1Reported = false;
#else
bool temp1Reported = true;
#endif

#ifdef NODE_2
HomieNode dhtNode2(NODE_2, "dht");
DHT dht2(DHT_PIN_2, DHT_TYPE);
bool temp2Reported = false;
#else
bool temp2Reported = true;
#endif


/**
 * Report the battery voltage.
 */
void reportVoltage()
{
    int voltageCount = analogRead(VOLTAGE_PIN);
    float voltage = VOLTAGE_COEFFICIENT * voltageCount;
    Homie.setNodeProperty(batteryNode, "voltage", String(voltage));
}

/**
 * Read the DHT sensor and report the data.  Keep trying on subsequent calls
 * until we successfully report.
 */
void reportSensorData(DHT &dht, HomieNode &dhtNode, bool &tempReported)
{
  if( ! tempReported ) {
    Serial.println("Attempting to read temperature");

    float tempC = dht.readTemperature();
    Serial.print("tempC ");  Serial.println(tempC);
    float tempF = dht.readTemperature(true);
    Serial.print("tempF ");  Serial.println(tempF);
    float humidity = dht.readHumidity();
    Serial.print("humididy ");  Serial.println(humidity);

    if( ! isnan(tempC) && ! isnan(tempF) && ! isnan(humidity) ) {
      float heatIndex = dht.computeHeatIndex(tempF, humidity);
      Homie.setNodeProperty(dhtNode, "tempC", String(tempC));
      Homie.setNodeProperty(dhtNode, "tempF", String(tempF));
      Homie.setNodeProperty(dhtNode, "humidity", String(humidity));
      Homie.setNodeProperty(dhtNode, "heatIndex", String(heatIndex));

      tempReported = true;
      Serial.println("Reported temperature");
    }
  }
}

/**
 * Called once when Homie is connected and ready.
 */
void setupHandler()
{
  #ifdef NODE_1
  dht1.begin();
  #endif

  #ifdef NODE_2
  dht2.begin();
  #endif

  reportVoltage();
}

/**
 * Looped when homie is connected and ready.
 */
void loopHandler()
{
  #ifdef NODE_1
  reportSensorData(dht1, dhtNode1, temp1Reported);
  #endif

  #ifdef NODE_2
  reportSensorData(dht2, dhtNode2, temp2Reported);
  #endif

  if( temp1Reported && temp2Reported ) {
    // disconnect MQTT, which will trigger deep sleep when complete
    Serial.println("Temps reported successfully; disconnecting");
    Homie.disconnectMqtt();
  }
}

/**
 * Called when Homie transitions between states
 */
void eventHandler(HomieEvent event) {
  switch(event) {
    case HOMIE_MQTT_DISCONNECTED:
      ESP.deepSleep(DEEP_SLEEP_SECONDS * 1000000);
      delay(1000); // allow deep sleep to occur
      break;
  }
}

void setup()
{
  Serial.begin(115200);

  Homie_setFirmware("tempsensor", "1.0.0");
  Homie_setBrand("clough42");

  Homie.disableResetTrigger();
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(eventHandler);
  Homie.setup();
}

void loop()
{
  Homie.loop();
}
