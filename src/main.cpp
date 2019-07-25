#define ARDUINO_RUNNING_CORE 0

#include <Arduino.h>
#include "globalVar.h"
#include "tft.cpp"
#include "mqtt.cpp"
// Include the libraries we need


void TaskTemp(void *pvParameters);
void TaskBlink( void *pvParameters );
void TaskPrintTemp(void *pvParameters);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(100);
  tftSetup();
  setupMqtt();

  sensors.begin();
  
  xTaskCreate( TaskTemp, "task temp", 2000, NULL, 2, NULL);
  //xTaskCreatePinnedToCore( TaskBlink, "task blink", configMINIMAL_STACK_SIZE, NULL, 2, NULL, 0);
}

void TaskTemp(void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    sensors.requestTemperatures();    
    // for(int i = 0; i < 5; i++)
    // {
    //   Serial.println(sensors.getTempCByIndex(i));
    // }
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