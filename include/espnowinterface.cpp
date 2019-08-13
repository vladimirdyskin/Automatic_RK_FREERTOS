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
        NotIdent = 0,
        TempSensor = 1,
        PlateDriver = 2
    };
    uint8_t macAddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    typeDevices typeDevice = NotIdent;
    enum answerCommand
    {
        OK = 41,
        FAIL = 42
    };
    bool waitAnswer = false;
    //bool init = false;
};

sensorsStruct sensorsEspNow[20];
int countSensorsEspNow = 0;

struct __attribute__((packed)) msgStruct
{
    sensorsStruct::typeDevices device;
    enum msgTypes
    {
        Data,
        Connect,
        Command,
        Answer
    } msgType;
    char valueData[20];
};
msgStruct myMsgSensor;

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
        NONE = 11,
        POWER_ON = 12,
        POWER_OFF = 13,
        SET_POWER_200 = 14,
        SET_POWER_500 = 15,
        SET_POWER_800 = 16,
        SET_POWER_1000 = 17,
        SET_POWER_1300 = 18,
        SET_POWER_1600 = 19,
        SET_POWER_1800 = 20,
        SET_POWER_2000 = 21,
    };
    //commandList command;
};
dataStructPlateDriver myDataPlateDriver;

void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void onSendEspNowCallBack(const uint8_t *mac_addr, esp_now_send_status_t status);
int searchSensorEspNow(const uint8_t *mac_addr);
void addSensorEspNow(const uint8_t *mac_addr, sensorsStruct::typeDevices typeDevice);
void sendCommandPowerOn(bool On);
void sendCommandSetPower(int Power);
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
    Serial.println(WiFi.macAddress());
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

    memcpy(&myMsgSensor, data, data_len);

    int indexCurrent = searchSensorEspNow(mac_addr);
    if (indexCurrent == -1)
    {
        Serial.println("Not Found Esp sensor");
        if (myMsgSensor.msgType == msgStruct::msgTypes::Connect)
        {
            addSensorEspNow(mac_addr, myMsgSensor.device);
            if (myMsgSensor.device == sensorsStruct::typeDevices::PlateDriver)
            {
                indexPlateDrive = countSensorsEspNow - 1;
                myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
                myMsgSensor.msgType = msgStruct::msgTypes::Connect;
                strcpy(myMsgSensor.valueData, "Hi");
            }
            uint8_t bsSend[sizeof(myMsgSensor)];
            memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
            ESPNow.send_message(sensorsEspNow[countSensorsEspNow - 1].macAddr, bsSend, sizeof(bsSend));
        }
    }
    else
    {
        if (myMsgSensor.msgType == msgStruct::msgTypes::Connect)
        {
            if (myMsgSensor.device == sensorsStruct::typeDevices::PlateDriver)
            {
                int currentState = atoi(myMsgSensor.valueData);
                // if (strcmp(myMsgSensor.valueData, "Notinit") != 0)
                // {
                //     myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
                //     myMsgSensor.msgType = msgStruct::msgTypes::Connect;
                //     strcpy(myMsgSensor.valueData, "Hi");
                //     uint8_t bsSend[sizeof(myMsgSensor)];
                //     memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
                //     ESPNow.send_message(sensorsEspNow[countSensorsEspNow - 1].macAddr, bsSend, sizeof(bsSend));
                //     Serial.println("Plate reinit");
                // }
                // else
                // {
                switch (currentState)
                {
                case dataStructPlateDriver::commandList::NONE:
                    myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
                    myMsgSensor.msgType = msgStruct::msgTypes::Connect;
                    strcpy(myMsgSensor.valueData, "Hi");
                    uint8_t bsSend[sizeof(myMsgSensor)];
                    memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
                    ESPNow.send_message(sensorsEspNow[indexCurrent].macAddr, bsSend, sizeof(bsSend));
                    Serial.println("Plate reinit");
                    break;
                case dataStructPlateDriver::commandList::POWER_OFF:
                    Serial.println("POWER IS OFF");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_200:
                    Serial.println("POWER IS 200");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_500:
                    Serial.println("POWER IS 500");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_800:
                    Serial.println("POWER IS 800");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1000:
                    Serial.println("POWER IS 1000");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1300:
                    Serial.println("POWER IS 1300");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1600:
                    Serial.println("POWER IS 1600");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1800:
                    Serial.println("POWER IS 1800");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_2000:
                    Serial.println("POWER IS 2000");
                    break;
                }
                Serial.println(currentState);
                Serial.println("Plate is online");

                //swit
                // }
            }
        }
        if (myMsgSensor.msgType == msgStruct::msgTypes::Answer)
        {
            if (myMsgSensor.device == sensorsStruct::typeDevices::PlateDriver)
            {
                int commandResult = atoi(myMsgSensor.valueData);
                if (commandResult == sensorsStruct::answerCommand::OK)
                {
                    Serial.println("Command succes");
                }
                if (commandResult == sensorsStruct::answerCommand::FAIL)
                {
                    Serial.println("Command fail");
                }
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

void sendCommandPowerOn(bool On)
{
    if (indexPlateDrive == -1)
        return;
    if (On)
    {
        itoa(dataStructPlateDriver::POWER_ON, myMsgSensor.valueData, 10);
        Serial.println("Send Power On");
    }
    else
    {
        itoa(dataStructPlateDriver::POWER_OFF, myMsgSensor.valueData, 10);
        Serial.println("Send Power Off");
    }
    sensorsEspNow[indexPlateDrive].waitAnswer = true;
    myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
    myMsgSensor.msgType = msgStruct::msgTypes::Command;
    uint8_t bsSend[sizeof(myMsgSensor)];
    memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
    ESPNow.send_message(sensorsEspNow[indexPlateDrive].macAddr, bsSend, sizeof(bsSend));
}

void sendCommandSetPower(int Power)
{
    switch (Power)
    {
    case 200:
        itoa(dataStructPlateDriver::SET_POWER_200, myMsgSensor.valueData, 10);
        break;
    case 500:
        itoa(dataStructPlateDriver::SET_POWER_500, myMsgSensor.valueData, 10);
        break;
    case 800:
        itoa(dataStructPlateDriver::SET_POWER_800, myMsgSensor.valueData, 10);
        break;
    case 1000:
        itoa(dataStructPlateDriver::SET_POWER_1000, myMsgSensor.valueData, 10);
        break;
    case 1300:
        itoa(dataStructPlateDriver::SET_POWER_1300, myMsgSensor.valueData, 10);
        break;
    case 1600:
        itoa(dataStructPlateDriver::SET_POWER_1600, myMsgSensor.valueData, 10);
        break;
    case 1800:
        itoa(dataStructPlateDriver::SET_POWER_1800, myMsgSensor.valueData, 10);
        break;
    case 2000:
        itoa(dataStructPlateDriver::SET_POWER_2000, myMsgSensor.valueData, 10);
        break;
    }
    sensorsEspNow[indexPlateDrive].waitAnswer = true;
    myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
    myMsgSensor.msgType = msgStruct::msgTypes::Command;
    uint8_t bsSend[sizeof(myMsgSensor)];
    memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
    ESPNow.send_message(sensorsEspNow[indexPlateDrive].macAddr, bsSend, sizeof(bsSend));
    Serial.println("Send Set Power");
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
    ESPNow.add_peer(sensorsEspNow[countSensorsEspNow].macAddr);
    countSensorsEspNow++;
    Serial.println("Add sensor");
}