#include <WiFi.h>
#include <WiFiClient.h>

#include <PubSubClient.h>
#include <string.h>
#define TOKEN "BBFF-gChPC9q5KqfjEU2LYyjbAKotTlyKcm" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "ESP32" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif ESP32
#include <WiFi.h>
#endif
#include "ESPNowW.h"
uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

struct __attribute__((packed)) DataStruct {
    // char text[32];
    float temp;
};
DataStruct myData;

/****************************************
 * Define Constants
 ****************************************/
#define TEMP1 "Temp1" // Assing the variable label
#define TEMP2 "Temp2" // Assing the variable label
#define DEVICE_LABEL "rk" // Assig the device label

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
// Space to store values to send
char str_sensor[10];

extern lv_obj_t *win, *txtIpAdress, *txtSignalStrech; 
extern float tempValue[5];

WiFiClient ubidots;
PubSubClient client(ubidots);

#define WIFI_SSID "Jewel"
#define WIFI_PASSWORD "lscrbydjdf"

void TaskMqtt(void *pvParameters);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void WiFiEvent(WiFiEvent_t event);
void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len);

void setupMqtt() 
{
 
   // delete old config
    WiFi.disconnect(true);
    delay(1000);
    WiFi.onEvent(WiFiEvent);

 #ifdef ESP8266
    WiFi.mode(WIFI_STA); // MUST NOT BE WIFI_MODE_NULL
  #elif ESP32
    WiFi.mode(WIFI_MODE_APSTA);
  #endif
    ESPNow.set_mac(mac);
    WiFi.disconnect();
    if(ESPNow.init() == ESP_OK)
    {
      Serial.println("Esp now ok");
    }
    else
    {
      Serial.println("Esp now ok");
    }
    
    ESPNow.reg_recv_cb(onRecvEspNow);    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println();
    Serial.println();
    Serial.println("Wait for WiFi... ");
    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);
    xTaskCreate( TaskMqtt, "task mqtt", 2000, NULL, 2, NULL);
}

void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len) 
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
             mac_addr[5]);
    Serial.print("Last Packet Recv from: ");
    Serial.println(macStr);
    Serial.print("Last Packet Recv Data: ");
    // if it could be a string, print as one
    if (data[data_len - 1] == 0)
        Serial.printf("%s\n", data);
    // additionally print as hex
    for (int i = 0; i < data_len; i++) {
        Serial.printf("%x ", data[i]);
    }
    Serial.println("");
    memcpy(&myData, data, sizeof(myData));
    Serial.println(myData.temp);
    tempValue[1] = myData.temp;
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
  if(!client.connected()) return;
  
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", TEMP1); // Adds the variable label        

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(tempValue[0], 4, 2, str_sensor);        
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value
  client.publish(topic, payload);

  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", TEMP2); // Adds the variable label        

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(tempValue[1], 4, 2, str_sensor);        
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value
  client.publish(topic, payload);

  Serial.println("Publishing data to Ubidots Cloud");
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
  while (!client.connected() && WiFi.isConnected()) 
  {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) 
    {
      Serial.println("Connected");
    } 
    else
    {
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
