#define ARDUINO_RUNNING_CORE 0

#include <Arduino.h>
#include <tft.cpp>


// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 26
#define TEMPERATURE_PRECISION 12
OneWire oneWireT(ONE_WIRE_BUS);
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensor(&oneWireT);
// arrays to hold device addresses
DeviceAddress temp1, temp2, temp3;
  float t1 = 0, t2 = 0, t3 = 0;


void TaskTemp(void *pvParameters);
void TaskBlink( void *pvParameters );
void TaskPrintTemp(void *pvParameters);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  tftSetup();

  sensor.begin();
  sensor.getAddress(temp1, 0);
  sensor.getAddress(temp2, 1);
  sensor.getAddress(temp3, 2);
  sensor.setResolution(temp1, TEMPERATURE_PRECISION);
  sensor.setResolution(temp2, TEMPERATURE_PRECISION);
  sensor.setResolution(temp3, TEMPERATURE_PRECISION);


  //xTaskCreatePinnedToCore( TaskBlink, "task blink", configMINIMAL_STACK_SIZE, NULL, 2, NULL, 0);
    xTaskCreate( TaskTemp, "task temp", 2048, NULL, 2, NULL);
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

void TaskTemp(void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    sensor.requestTemperatures();
    t1 = sensor.getTempCByIndex(0);
    t2 = sensor.getTempCByIndex(1);
    t3 = sensor.getTempCByIndex(2);
    Serial.println(t1);
    lcdT1 = t1;
    vTaskDelay(3000);
  }
}