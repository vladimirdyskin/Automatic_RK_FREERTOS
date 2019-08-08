#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif ESP32
#include <WiFi.h>
#endif
#include <esp_now.h>
#include "ESPNowW.h"

struct sensorsStruct
{
    enum typeDevices
    {
        NotIdent,
        TempSensor,
        PlateDriver
    };
    uint8_t macAddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    typeDevices typeDevice = NotIdent;
    enum answerCommand
    {
        OK,
        FAIL
    };
    bool waitAnswer = false;
    //bool init = false;
};

sensorsStruct sensorsEspNow[20];
int countSensorsEspNow = 0;

struct __attribute__((packed)) dataStructTempSensor
{
    float batVoltage;
    float temp;
};
dataStructTempSensor myDataTempSensor;

struct __attribute__((packed)) dataStructPlateDriver
{
    enum commandList
    {
        NONE,
        POWER_ON,
        POWER_OFF,
        SET_POWER
    };
    //commandList command;
};
dataStructPlateDriver myDataPlateDriver;

void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void onSendEspNowCallBack(const uint8_t *mac_addr, esp_now_send_status_t status);
int searchSensorEspNow(const uint8_t *mac_addr);
void addSensorEspNow(const uint8_t *mac_addr, sensorsStruct::typeDevices typeDevice);
void sendCommandPowerOn();

int indexPlateDrive = -1;

void setupEspNow()
{
#ifdef ESP8266
    WiFi.mode(WIFI_STA); // MUST NOT BE WIFI_MODE_NULL
#elif ESP32
    WiFi.mode(WIFI_MODE_APSTA);
#endif

    WiFi.disconnect();
    if (ESPNow.init() == ESP_OK)
    {
        Serial.println(PSTR("Esp now ok"));
    }
    else
    {
        Serial.println(PSTR("Esp not connect"));
    }
    //Serial.println(WiFi.macAddress());
    ESPNow.reg_recv_cb(onRecvEspNow);
    ESPNow.reg_send_cb(onSendEspNowCallBack);
}

void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
             mac_addr[5]);
    Serial.print(PSTR("Last Packet Recv from: "));
    Serial.println(macStr);
    Serial.print(PSTR("Last Packet Recv Data: "));
    // if it could be a string, print as one
    if (data[data_len - 1] == 0)
        Serial.printf("%s\n", data);
    // additionally print as hex
    for (int i = 0; i < data_len; i++)
    {
        Serial.printf("%x ", data[i]);
    }

    int indexCurrent = searchSensorEspNow(mac_addr);
    if (indexCurrent == -1)
    {
        Serial.println("Not Found Esp sensor");
        addSensorEspNow(mac_addr, sensorsStruct::NotIdent);
    }
    else
    {
        Serial.println("Esp sensor exist");
        if (sensorsEspNow[indexCurrent].typeDevice == sensorsStruct::NotIdent)
        {
            Serial.println("Esp sensor not ident");
            if (data[0] == sensorsStruct::PlateDriver)
            {
                Serial.println("Esp sensor is Plate");
                sensorsEspNow[indexCurrent].typeDevice = sensorsStruct::PlateDriver;
                indexPlateDrive = indexCurrent;
                uint8_t bsSend[1] = {sensorsStruct::PlateDriver};
                ESPNow.send_message(sensorsEspNow[indexCurrent].macAddr, bsSend, sizeof(bsSend));
                Serial.println("Send Im Here");
            }
        }
        if (sensorsEspNow[indexCurrent].waitAnswer)
        {
            if (sensorsEspNow[indexCurrent].typeDevice == sensorsStruct::typeDevices::PlateDriver)
            {
                Serial.print("Plate drive  -> ");
            }
            if (data[0] == sensorsStruct::answerCommand::OK)
            {
                Serial.println("Succes command");
            }
            if (data[0] == sensorsStruct::answerCommand::FAIL)
            {
                Serial.println("Fail command");
            }
            sensorsEspNow[indexCurrent].waitAnswer = false;
        }
        else
        {
            if (sensorsEspNow[indexCurrent].typeDevice == sensorsStruct::typeDevices::PlateDriver)
            {
                Serial.print("Plate drive  -> online");
            }
        }
    }

    // myDataTempSensor.temp = 5;
    // uint8_t bsSend[sizeof(myDataTempSensor)];
    // memcpy(bsSend, &myDataTempSensor, sizeof(myDataTempSensor));
    // Serial.println(esp_err_to_name(ESPNow.send_message(mac, bsSend, sizeof(myDataTempSensor))));
}

void onSendEspNowCallBack(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Sent to: ");
    Serial.println(macStr);
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == 0 ? "Delivery Success" : "Delivery Fail");
}

void sendCommandPowerOn()
{
    if (indexPlateDrive == -1)
        return;
    sensorsEspNow[indexPlateDrive].waitAnswer = true;
    uint8_t bsSend[1] = {dataStructPlateDriver::commandList::POWER_ON};
    ESPNow.send_message(sensorsEspNow[indexPlateDrive].macAddr, bsSend, sizeof(bsSend));
    Serial.println("Send Power On");
}

int searchSensorEspNow(const uint8_t *mac_addr)
{
    bool searchOk = false;
    for (int i = 0; i < countSensorsEspNow; i++)
    {
        searchOk = false;
        for (int x = 0; x < 6; x++)
        {
            if (sensorsEspNow[i].macAddr[x] == mac_addr[x])
            {
                searchOk = true;
            }
            else
            {
                searchOk = false;
                break;
            }
        }
        if (searchOk)
        {
            return i;
        }
    }
    return -1;
}

void addSensorEspNow(const uint8_t *mac_addr, sensorsStruct::typeDevices typeDevice)
{
    if (countSensorsEspNow > 20)
        return;
    for (int x = 0; x < 6; x++)
    {
        sensorsEspNow[countSensorsEspNow].macAddr[x] = mac_addr[x];
    }
    sensorsEspNow[countSensorsEspNow].typeDevice = typeDevice;
    //sensorsEspNow[countSensorsEspNow].init = true;
    ESPNow.add_peer(sensorsEspNow[countSensorsEspNow].macAddr);
    countSensorsEspNow++;
}