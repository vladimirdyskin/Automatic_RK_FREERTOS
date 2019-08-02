#define ARDUINO_RUNNING_CORE 0

#include <Arduino.h>
#include "tft.cpp"
#include "mqtt.cpp"
//#include "ESPUI.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 26
#define TEMPERATURE_PRECISION 12
OneWire oneWireT(ONE_WIRE_BUS);
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWireT);
// arrays to hold device addresses
// Include the libraries we need


void TaskTemp(void *pvParameters);
void TaskBlink( void *pvParameters );
void TaskPrintTemp(void *pvParameters);
float tempValue[5];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(100);
  tftSetup();
  setupMqtt();

  sensors.setWaitForConversion(false);
  sensors.begin();
  xTaskCreate( TaskTemp, "task temp", 2000, NULL, 2, NULL);
  //xTaskCreatePinnedToCore( TaskBlink, "task blink", configMINIMAL_STACK_SIZE, NULL, 2, NULL, 0);

//  ESPUI.begin("OK");
}

void TaskTemp(void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    sensors.requestTemperatures();
    tempValue[0] = sensors.getTempCByIndex(0);
    vTaskDelay(3000);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(1000);  // one tick delay (15ms) in between reads for stability
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay(1000);  // one tick delay (15ms) in between reads for stability
  }
}