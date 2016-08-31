#ifndef __TempSensor_H
#define __TempSensor_H

#include <DHT.h>
#include <Homie.h>

class TempSensor {

public:
  TempSensor(int pin);
  ~TempSensor();

  void read();

  float getTempC() { return _tempC; }
  float getTempF() { return _tempF; }
  float getHumidity() { return _humidity; }

private:

  float _tempC;
  float _tempF;
  float _humidity;

  DHT _dht;
  int _pin;

};

#endif
