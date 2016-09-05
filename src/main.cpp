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
#include <Homie.h>

#include "TempSensor.hpp"

// Homie node names - Comment out either to disable
#define NODE_1 "ambient"
//#define NODE_2 "refrigerator"

#define DHT_PIN_1 D4
#define DHT_PIN_2 D3

#define VOLTAGE_PIN A0

bool reported = false;
HomieNode batteryNode("battery", "voltage");

#ifdef NODE_1
HomieNode dhtNode1(NODE_1, "dht");
TempSensor tempSensor1(DHT_PIN_1);
#endif

#ifdef NODE_2
HomieNode dhtNode2(NODE_2, "dht");
TempSensor tempSensor2(DHT_PIN_2);
#endif

HomieSetting<double> voltageCoefficientSetting("voltage_coefficient", "ADC scaling factor for battery voltage");
HomieSetting<long> deepSleepTimeSetting("deep_sleep_seconds", "Deep sleep time, in seconds, between readings");

/**
 * Report the battery voltage.
 */
void reportVoltage()
{
    int voltageCount = analogRead(VOLTAGE_PIN);
    float voltage = voltageCoefficientSetting.get() * voltageCount;
    Homie.setNodeProperty(batteryNode, "voltage").send(String(voltage));
}

/**
 * Read the DHT sensor and report the data.  Keep trying on subsequent calls
 * until we successfully report.
 */
void reportSensorData(TempSensor &sensor, HomieNode &dhtNode)
{
  Serial.println("Reporting temperature");
  Homie.setNodeProperty(dhtNode, "tempC").send(String(sensor.getTempC()));
  Homie.setNodeProperty(dhtNode, "tempF").send(String(sensor.getTempF()));
  Homie.setNodeProperty(dhtNode, "humidity").send(String(sensor.getHumidity()));
  Serial.println("Reported temperature");
}

/**
 * Called once when Homie is connected and ready.
 */
void setupHandler()
{

}

/**
 * Looped while Homie is operating.
 */
void loopHandler()
{
  if( ! reported ) {
    #ifdef NODE_1
    reportSensorData(tempSensor1, dhtNode1);
    #endif

    #ifdef NODE_2
    reportSensorData(tempSensor2, dhtNode2);
    #endif

    reportVoltage();

    reported = true;

    Homie.prepareForSleep();
  }
}

/**
 * Called when Homie transitions between states
 */
void eventHandler(HomieEvent event) {
  switch(event) {
    case HomieEvent::READY_FOR_SLEEP:
      Serial.println("Going to sleep...");
      ESP.deepSleep(deepSleepTimeSetting.get() * 1000000);
      break;
  }
}

void setup()
{
  Serial.begin(115200);

  Homie_setFirmware("tempsensor", "1.1.0");
  Homie_setBrand("clough42");

  Homie
    .disableResetTrigger()
    .setSetupFunction(setupHandler)
    .onEvent(eventHandler)
    .setLoopFunction(loopHandler)
    .setup();

  #ifdef NODE_1
  tempSensor1.read();
  #endif

  #ifdef NODE_2
  tempSensor2.read();
  #endif
}

void loop()
{
  Homie.loop();
}
