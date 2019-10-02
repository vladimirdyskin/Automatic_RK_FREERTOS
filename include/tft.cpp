#include "tft_callibration.cpp"
#include "tft_interface.cpp"
#include "Arduino.h"
#include <WiFi.h>
#include "ESPNowW.h"

enum but
{
    butCloseMain,
    butSettingsMain,
    butSettingsWiFiMain,
    butCloseSettingsWiFi,
    butCloseRect,
    butCloseSettings
};

const int countButtons = 10;
lv_obj_t *buttons[countButtons];

lv_obj_t *buttonSendPowerOn, *buttonSendPowerOff, *buttonRect, *buttonTelo;
lv_obj_t *switchFlow;

lv_obj_t *win;
lv_obj_t *txtTemp[4], *txtIpAdress, *txtSignalStrech, *txtState, *imgPower;
lv_obj_t *screenMain, *screenRect, *screenSettingsWiFi, *screenSettings;
extern float tempValue[5];
extern void publishValueMqtt();

void sendCommandSetPower(int Power);
extern void sendCommandPowerOn(bool On);

extern void RectStart();
extern int stateRect;
extern void sendCommandFlowOpen(bool On);

void lv_ex_win_main(void);
void lv_ex_win_1_peregon(void);
void lv_ex_win_settings(void);
void lv_ex_win_settingswifi(void);
void TaskUpdateValue(void *pvParameters);

void tftSetup()
{
    InitGraphics();

    lv_ex_win_main();
    lv_ex_win_1_peregon();
    lv_ex_win_settings();
    lv_ex_win_settingswifi();

    vTaskDelay(1000);
    lv_scr_load(screenMain);

    xTaskCreate(TaskUpdateValue, "task update value", 8000, NULL, 3, NULL);
}

void TaskUpdateValue(void *pvParameters)
{
    (void)pvParameters;

    static float pLcdT = 0;
    static float pLcd2 = 0;
    for (;;)
    {
        if ((pLcdT != tempValue[0]) && (txtTemp[0] != 0))
        {
            char buf[8];
            snprintf(buf, 8, "%.2f", tempValue[0]);
            lv_label_set_text(txtTemp[0], buf);
            publishValueMqtt();
        }
        pLcdT = tempValue[0];
        if ((pLcd2 != tempValue[1]) && (txtTemp[1] != 0))
        {
            char buf[8];
            snprintf(buf, 8, "%.2f", tempValue[1]);
            lv_label_set_text(txtTemp[1], buf);
            publishValueMqtt();
        }
        pLcd2 = tempValue[1];

        if (lv_scr_act() == screenSettingsWiFi)
        {
            lv_label_set_text(txtIpAdress, WiFi.localIP().toString().c_str());

            char buf2[8];
            sprintf(buf2, "%d", WiFi.RSSI());
            lv_label_set_text(txtSignalStrech, buf2);
        }
        vTaskDelay(1000);
    }
}

static void my_event_cb(lv_obj_t *obj, lv_event_t event)
{
    switch (event)
    {
    case LV_EVENT_PRESSED:
        if (obj == buttons[but::butCloseMain])
        {
            lv_scr_load(screenRect);
        }
        if (obj == buttons[but::butCloseRect])
        {
            lv_scr_load(screenMain);
        }
        if (obj == buttons[but::butCloseSettingsWiFi])
        {
            lv_scr_load(screenMain);
        }
        if (obj == buttons[but::butSettingsWiFiMain])
        {
            lv_scr_load(screenSettingsWiFi);
        }
        if (obj == buttons[but::butSettingsMain])
        {
            lv_scr_load(screenSettings);
        }
        if (obj == buttons[but::butCloseSettings])
        {
            lv_scr_load(screenMain);
        }
        if (obj == buttonSendPowerOff)
        {
            sendCommandPowerOn(false);
        }
        if (obj == buttonSendPowerOn)
        {
            sendCommandPowerOn(true);
        }
        if(obj == buttonRect)
        {
            RectStart();
        }
        if(obj == buttonTelo)
        {
            stateRect = 3;
        }
        break;

    case LV_EVENT_SHORT_CLICKED:
        break;

    case LV_EVENT_CLICKED:
        break;

    case LV_EVENT_LONG_PRESSED:
        break;

    case LV_EVENT_LONG_PRESSED_REPEAT:
        break;

    case LV_EVENT_RELEASED:
        break;
    case LV_EVENT_VALUE_CHANGED:
        if(obj == switchFlow)
        {
            //Serial.println(printf("State: %s\n", lv_sw_get_state(switchFlow) ? "On" : "Off"));
            sendCommandFlowOpen(lv_sw_get_state(switchFlow));
        }
        break;        
    } /*Etc.*/
}

static const char *btnm_map[] = {"200", "500", "800", "1000", "\n",
                                 "1300", "1600", "1800", "2000", ""};

static void event_powerChange(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_btnm_get_active_btn_text(obj);
        int powerValue = atoi(txt);
        sendCommandSetPower(powerValue);

        //printf("%s was pressed\n", txt);
    }
}

void lv_ex_win_main(void)
{
    screenMain = lv_obj_create(NULL, NULL);
    /*Create a window*/
    win = lv_win_create(screenMain, NULL);
    lv_win_set_title(win, " RK System ");
    /*Add control button to the header*/
    // buttons[but::butCloseMain] = lv_win_add_btn(win, LV_SYMBOL_CLOSE);           /*Add close button and use built-in close action*/
    // lv_obj_set_event_cb(buttons[but::butCloseMain], my_event_cb);
    buttons[but::butSettingsMain] = lv_win_add_btn(win, LV_SYMBOL_SETTINGS); /*Add a setup button*/
    lv_obj_set_event_cb(buttons[but::butSettingsMain], my_event_cb);
    buttons[but::butSettingsWiFiMain] = lv_win_add_btn(win, LV_SYMBOL_WIFI);
    lv_obj_set_event_cb(buttons[but::butSettingsWiFiMain], my_event_cb);

    /*Set the title*/
    /*Add some dummy content*/
    lv_obj_t *label = lv_label_create(win, NULL);
    lv_label_set_text(label, PSTR("Temp1"));
    lv_obj_set_pos(label, 30, 30);

    txtTemp[0] = lv_label_create(win, NULL);
    lv_label_set_text(txtTemp[0], "");
    lv_obj_set_pos(txtTemp[0], 100, 30);

    label = lv_label_create(win, NULL);
    lv_label_set_text(label, PSTR("Temp2"));
    lv_obj_set_pos(label, 30, 60);

    txtTemp[1] = lv_label_create(win, NULL);
    lv_label_set_text(txtTemp[1], "");
    lv_obj_set_pos(txtTemp[1], 100, 60);

    buttonSendPowerOn = lv_btn_create(win, NULL);
    lv_obj_t *img1 = lv_img_create(buttonSendPowerOn, NULL);
    lv_img_set_src(img1, LV_SYMBOL_POWER);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_event_cb(buttonSendPowerOn, my_event_cb);
    lv_obj_set_pos(buttonSendPowerOn, 50, 90);
    lv_obj_set_size(buttonSendPowerOn, 50, 40);

    buttonSendPowerOff = lv_btn_create(win, NULL);
    img1 = lv_img_create(buttonSendPowerOff, NULL);
    lv_img_set_src(img1, LV_SYMBOL_STOP);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_event_cb(buttonSendPowerOff, my_event_cb);
    lv_obj_set_pos(buttonSendPowerOff, 120, 90);
    lv_obj_set_size(buttonSendPowerOff, 50, 40);

    buttonRect = lv_btn_create(win, NULL);
    img1 = lv_img_create(buttonRect, NULL);
    lv_img_set_src(img1, LV_SYMBOL_REFRESH);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_event_cb(buttonRect, my_event_cb);
    lv_obj_set_pos(buttonRect, 50, 150);
    lv_obj_set_size(buttonRect, 50, 40);


    buttonTelo = lv_btn_create(win, NULL);
    img1 = lv_img_create(buttonTelo, NULL);
    lv_img_set_src(img1, LV_SYMBOL_UPLOAD);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_event_cb(buttonTelo, my_event_cb);
    lv_obj_set_pos(buttonTelo, 120, 150);
    lv_obj_set_size(buttonTelo, 50, 40);

    switchFlow = lv_sw_create(win, NULL);
    lv_obj_set_pos(switchFlow, 120, 200);
    lv_obj_set_size(switchFlow, 70, 40);
    lv_obj_set_event_cb(switchFlow, my_event_cb);

    txtState = lv_label_create(win, NULL);
    lv_label_set_text(txtState, "");
    lv_obj_set_pos(txtState, 250, 150);

    lv_obj_t *btnm1 = lv_btnm_create(win, NULL);
    lv_btnm_set_map(btnm1, btnm_map);
    //lv_btnm_set_btn_width(btnm1, 10, 2); /*Make "Action1" twice as wide as "Action2"*/
    lv_obj_align(btnm1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(btnm1, event_powerChange);
    lv_obj_set_pos(btnm1, 250, 10);
    lv_obj_set_size(btnm1, 200, 100);
}

void lv_ex_win_settingswifi(void)
{
    screenSettingsWiFi = lv_obj_create(NULL, NULL);
    /*Create a window*/
    lv_obj_t *win = lv_win_create(screenSettingsWiFi, NULL);
    lv_win_set_title(win, PSTR("Settings WIFI")); /*Set the title*/

    /*Add control button to the header*/
    lv_obj_t *close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE); /*Add close button and use built-in close action*/
    buttons[but::butCloseSettingsWiFi] = close_btn;
    lv_obj_set_event_cb(close_btn, my_event_cb);

    lv_obj_t *label = lv_label_create(win, NULL);
    lv_obj_set_pos(label, 30, 30);
    lv_label_set_text(label, PSTR("IP: "));

    txtIpAdress = lv_label_create(win, NULL);
    lv_obj_set_pos(txtIpAdress, 100, 30);
    lv_label_set_text(txtIpAdress, PSTR("0.0.0.0"));

    label = lv_label_create(win, NULL);
    lv_obj_set_pos(label, 30, 50);
    lv_label_set_text(label, PSTR("Signal:"));

    txtSignalStrech = lv_label_create(win, NULL);
    lv_obj_set_pos(txtSignalStrech, 100, 50);
    lv_label_set_text(txtSignalStrech, PSTR("NA"));
}

void lv_ex_win_settings(void)
{
    screenSettings = lv_obj_create(NULL, NULL);
    /*Create a window*/
    lv_obj_t *win = lv_win_create(screenSettings, NULL);
    lv_win_set_title(win, PSTR("Settings")); /*Set the title*/

    /*Add control button to the header*/
    buttons[but::butCloseSettings] = lv_win_add_btn(win, LV_SYMBOL_CLOSE); /*Add close button and use built-in close action*/
    lv_obj_set_event_cb(buttons[but::butCloseSettings], my_event_cb);

    lv_obj_t *label = lv_label_create(win, NULL);
    lv_obj_set_pos(label, 30, 30);
    lv_label_set_text(label, PSTR("Settings: "));

    // txtIpAdress = lv_label_create(win, NULL);
    // lv_obj_set_pos(txtIpAdress, 100, 30);
    // lv_label_set_text(txtIpAdress, "0.0.0.0");

    // label = lv_label_create(win, NULL);
    // lv_obj_set_pos(label, 30, 50);
    // lv_label_set_text(label, "Signal:");

    // txtSignalStrech = lv_label_create(win, NULL);
    // lv_obj_set_pos(txtSignalStrech, 100, 50);
    // lv_label_set_text(txtSignalStrech, "0.0.0.0");
}

void lv_ex_win_1_peregon(void)
{
    screenRect = lv_obj_create(NULL, NULL);
    /*Create a window*/
    lv_obj_t *win = lv_win_create(screenRect, NULL);
    lv_win_set_title(win, PSTR("1 peregon")); /*Set the title*/

    /*Add control button to the header*/
    lv_obj_t *close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE); /*Add close button and use built-in close action*/
    buttons[but::butCloseRect] = close_btn;
    lv_obj_set_event_cb(close_btn, my_event_cb);
    lv_win_add_btn(win, LV_SYMBOL_SETTINGS); /*Add a setup button*/

    /*Add content to the tabs*/
    lv_obj_t *label = lv_label_create(win, NULL);
    lv_label_set_text(label, PSTR("\nThis the first win\n\n"
                                  "1 Peregon\n"
                                  "scrollable."));
}
