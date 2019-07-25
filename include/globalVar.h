#ifndef globalVar_h
#define globalVar_h

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 26
#define TEMPERATURE_PRECISION 12
OneWire oneWireT(ONE_WIRE_BUS);
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWireT);
// arrays to hold device addresses

#endif
