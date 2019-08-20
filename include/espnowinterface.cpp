#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif ESP32
#include <WiFi.h>
#endif
#include <esp_now.h>
#include "ESPNowW.h"
#include "Arduino.h"

#define DEBUGESPNOW 0

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
extern float tempValue[5];

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
    int currentPower;
    bool currentState;
    int indexPlateDrive = -1;
    //commandList command;
};
dataStructPlateDriver myDataPlateDriver;

void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void onSendEspNowCallBack(const uint8_t *mac_addr, esp_now_send_status_t status);
int searchSensorEspNow(const uint8_t *mac_addr);
void addSensorEspNow(const uint8_t *mac_addr, sensorsStruct::typeDevices typeDevice);
void sendCommandPowerOn(bool On);
void sendCommandSetPower(int Power);

void TaskAutoOnOffControl(void *pvParameters);


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
    ESPNow.reg_recv_cb(onRecvEspNow);
    ESPNow.reg_send_cb(onSendEspNowCallBack);

    xTaskCreate(TaskAutoOnOffControl, "control onoff", 8000, NULL, 2, NULL);
}

void onRecvEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    if (DEBUGESPNOW)
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
        Serial.println(" ");
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
                myDataPlateDriver.indexPlateDrive = countSensorsEspNow - 1;
                myDataPlateDriver.currentState = false;
                myDataPlateDriver.currentPower = 0;

                myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
                myMsgSensor.msgType = msgStruct::msgTypes::Connect;
                strcpy(myMsgSensor.valueData, "Hi");
            }
            uint8_t bsSend[sizeof(myMsgSensor)];
            memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
            ESPNow.send_message(sensorsEspNow[countSensorsEspNow - 1].macAddr, bsSend, sizeof(bsSend));
        }
        if (myMsgSensor.device == sensorsStruct::typeDevices::TempSensor)
        {
            addSensorEspNow(mac_addr, myMsgSensor.device);
        }
    }
    else
    {
        if (myMsgSensor.msgType == msgStruct::msgTypes::Connect)
        {
            if (myMsgSensor.device == sensorsStruct::typeDevices::PlateDriver)
            {
                int currentState = atoi(myMsgSensor.valueData);
                switch (currentState)
                {
                case dataStructPlateDriver::commandList::NONE:
                    myDataPlateDriver.indexPlateDrive = countSensorsEspNow - 1;
                    myDataPlateDriver.currentState = false;
                    myDataPlateDriver.currentPower = 0;

                    myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
                    myMsgSensor.msgType = msgStruct::msgTypes::Connect;
                    strcpy(myMsgSensor.valueData, "Hi");
                    uint8_t bsSend[sizeof(myMsgSensor)];
                    memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
                    ESPNow.send_message(sensorsEspNow[indexCurrent].macAddr, bsSend, sizeof(bsSend));
                    Serial.println("Plate reinit");
                    break;
                case dataStructPlateDriver::commandList::POWER_OFF:
                    myDataPlateDriver.currentState = false;
                    myDataPlateDriver.currentPower = 0;
                    Serial.println("POWER IS OFF");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_200:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 200;
                    Serial.println("POWER IS 200");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_500:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 500;
                    Serial.println("POWER IS 500");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_800:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 800;
                    Serial.println("POWER IS 800");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1000:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 1000;
                    Serial.println("POWER IS 1000");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1300:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 1300;
                    Serial.println("POWER IS 1300");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1600:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 1600;
                    Serial.println("POWER IS 1600");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_1800:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 1800;
                    Serial.println("POWER IS 1800");
                    break;
                case dataStructPlateDriver::commandList::SET_POWER_2000:
                    myDataPlateDriver.currentState = true;
                    myDataPlateDriver.currentPower = 2000;
                    Serial.println("POWER IS 2000");
                    break;
                }
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
        if (myMsgSensor.msgType == msgStruct::msgTypes::Data)
        {
            if (myMsgSensor.device == sensorsStruct::typeDevices::TempSensor)
            {
                memcpy(&myDataTempSensor, &myMsgSensor.valueData, sizeof(myMsgSensor));
                tempValue[1] = myDataTempSensor.temp;
                Serial.print("Temp is ");
                Serial.println(tempValue[1]);
            }
        }
    }
}

void onSendEspNowCallBack(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (DEBUGESPNOW)
    {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        Serial.print("Last Packet Sent to: ");
        Serial.println(macStr);
        Serial.print("Last Packet Send Status: ");
        Serial.println(status == 0 ? "Delivery Success" : "Delivery Fail");
    }
    else
    {
        Serial.println(status == 0 ? "Delivery Success" : "Delivery Fail");
    }
    
}

void sendCommandPowerOn(bool On)
{
    if (myDataPlateDriver.indexPlateDrive == -1)
    {
        Serial.println("Not regist Plate");
        return;
    }
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
    sensorsEspNow[myDataPlateDriver.indexPlateDrive].waitAnswer = true;
    myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
    myMsgSensor.msgType = msgStruct::msgTypes::Command;
    uint8_t bsSend[sizeof(myMsgSensor)];
    memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
    ESPNow.send_message(sensorsEspNow[myDataPlateDriver.indexPlateDrive].macAddr, bsSend, sizeof(bsSend));
}

void TaskAutoOnOffControl(void *pvParameters)
{
    (void)pvParameters;
    int prevPower = 0;
    for (;;)
    {
        vTaskDelay(3600 * (1000 / portTICK_PERIOD_MS));
        if (myDataPlateDriver.currentState)
        {
            Serial.println("Run auto onoff");
            switch (myDataPlateDriver.currentPower)
            {
            case 200:
                sendCommandSetPower(500);
                prevPower = 200;
                break;
            case 500:
                sendCommandSetPower(200);
                prevPower = 500;
                break;
            case 800:
                sendCommandSetPower(500);
                prevPower = 800;
                break;
            case 1000:
                sendCommandSetPower(800);
                prevPower = 1000;
                break;
            case 1300:
                sendCommandSetPower(1000);
                prevPower = 1300;
                break;
            case 1600:
                sendCommandSetPower(1300);
                prevPower = 1600;
                break;
            case 1800:
                sendCommandSetPower(1600);
                prevPower = 1800;
                break;
            case 2000:
                sendCommandSetPower(1800);
                prevPower = 2000;
                break;
            }
        }
        vTaskDelay(2 * (1000 / portTICK_PERIOD_MS));
        if (myDataPlateDriver.currentState)
        {
            Serial.println("Return power");
            sendCommandSetPower(prevPower);
        }
    }
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
    sensorsEspNow[myDataPlateDriver.indexPlateDrive].waitAnswer = true;
    myMsgSensor.device = sensorsStruct::typeDevices::PlateDriver;
    myMsgSensor.msgType = msgStruct::msgTypes::Command;
    uint8_t bsSend[sizeof(myMsgSensor)];
    memcpy(bsSend, &myMsgSensor, sizeof(myMsgSensor));
    ESPNow.send_message(sensorsEspNow[myDataPlateDriver.indexPlateDrive].macAddr, bsSend, sizeof(bsSend));
    Serial.print("Send Set Power ");
    Serial.println(Power);
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
    Serial.print("Add sensor ");
    if (typeDevice == sensorsStruct::PlateDriver)
    {
        Serial.println("Plate");
    }
    if (typeDevice == sensorsStruct::TempSensor)
    {
        Serial.println("Temp");
    }
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Mac address - ");
    Serial.println(macStr);
}