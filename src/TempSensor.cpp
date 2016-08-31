#include "TempSensor.hpp"

#define DHT_TYPE 22


TempSensor::TempSensor(int pin)
  : _dht(pin, DHT_TYPE)
{
  _pin = pin;
  _dht.begin();
}

TempSensor::~TempSensor()
{

}

void TempSensor::read()
{
  Serial.print("Reading data: ");
  Serial.println(_pin);

  while( ! _dht.read(false) )
  {
    Serial.print("Waiting for data...");
    Serial.println(_pin);
    delay(2000);
  }

  _tempC = _dht.readTemperature(false);
  _tempF = _dht.readTemperature(true);
  _humidity = _dht.readHumidity();
}
