#include "globalVar.h"
#include <WiFi.h>
#include <WiFiClient.h>

#include <PubSubClient.h>
#include <string.h>
#define TOKEN "BBFF-gChPC9q5KqfjEU2LYyjbAKotTlyKcm" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "ESP32" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices

/****************************************
 * Define Constants
 ****************************************/
#define TEMP1 "temp1" // Assing the variable label
#define DEVICE_LABEL "rk" // Assig the device label

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
// Space to store values to send
char str_sensor[10];

extern lv_obj_t *win; 
extern float tempValue[5];

WiFiClient ubidots;
PubSubClient client(ubidots);

#define WIFI_SSID "Jewel"
#define WIFI_PASSWORD "lscrbydjdf"

void TaskMqtt(void *pvParameters);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void WiFiEvent(WiFiEvent_t event);


void setupMqtt() 
{
   // delete old config
    WiFi.disconnect(true);
    delay(1000);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println();
    Serial.println();
    Serial.println("Wait for WiFi... ");


    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);
    xTaskCreate( TaskMqtt, "task mqtt", 2000, NULL, 2, NULL);
}


void TaskMqtt(void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
        vTaskDelay(3000);
  }
}

void publishValueMqtt()
{
        sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
        sprintf(payload, "%s", ""); // Cleans the payload
        sprintf(payload, "{\"%s\":", TEMP1); // Adds the variable label        

        /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
        dtostrf(tempValue[0], 4, 2, str_sensor);        
        sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value
        Serial.println("Publishing data to Ubidots Cloud");
        client.publish(topic, payload);
}


void callback(char* topic, byte* payload, unsigned int length) 
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = 0;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      vTaskDelay(2000);
    }
  }
}

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event)
    {
      case SYSTEM_EVENT_STA_GOT_IP:
      {
          Serial.println("WiFi connected");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          String s1("RK System     IP " + WiFi.localIP().toString());

          char buf[s1.length()+1];
          s1.toCharArray(buf, s1.length()+1);
          lv_win_set_title(win, buf);
      }
      break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          break;
      case SYSTEM_EVENT_SCAN_DONE:break;
      case SYSTEM_EVENT_STA_START: break;
      case SYSTEM_EVENT_STA_STOP: break;
      case SYSTEM_EVENT_STA_CONNECTED: break;
      case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: break;
      case SYSTEM_EVENT_STA_LOST_IP: break;
      case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: break;
      case SYSTEM_EVENT_STA_WPS_ER_FAILED: break;
      case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT: break;
      case SYSTEM_EVENT_STA_WPS_ER_PIN: break;
      case SYSTEM_EVENT_AP_START: break;
      case SYSTEM_EVENT_AP_STOP: break;
      case SYSTEM_EVENT_AP_STACONNECTED: break;
      case SYSTEM_EVENT_AP_STADISCONNECTED: break;
      case SYSTEM_EVENT_AP_STAIPASSIGNED: break;
      case SYSTEM_EVENT_AP_PROBEREQRECVED: break;
      case SYSTEM_EVENT_GOT_IP6: break;
      case SYSTEM_EVENT_ETH_START: break;
      case SYSTEM_EVENT_ETH_STOP: break;
      case SYSTEM_EVENT_ETH_CONNECTED: break;
      case SYSTEM_EVENT_ETH_DISCONNECTED: break;
      case SYSTEM_EVENT_ETH_GOT_IP: break;
      case SYSTEM_EVENT_MAX: break;
      case SYSTEM_EVENT_WIFI_READY: break;
    }
}
