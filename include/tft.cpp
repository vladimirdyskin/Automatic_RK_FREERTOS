#include "tft_callibration.cpp"
#include "tft_interface.cpp"
#include "Arduino.h"
#include <WiFi.h>
#include "ESPNowW.h"

extern void sendCommandPowerOn();
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

lv_obj_t *buttonSendPowerOn;

lv_obj_t *win;
lv_obj_t *txtTemp[4], *txtIpAdress, *txtSignalStrech;
lv_obj_t *screenMain, *screenRect, *screenSettingsWiFi, *screenSettings;
extern float tempValue[5];
extern void publishValueMqtt();

void lv_ex_win_main(void);
void lv_ex_win_1_peregon(void);
void lv_ex_win_settings(void);
void lv_ex_win_settingswifi(void);
void TaskUpdateValue(void *pvParameters);
static void my_event_poweron(lv_obj_t *obj, lv_event_t event);

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
    } /*Etc.*/
}

static void my_event_poweron(lv_obj_t *obj, lv_event_t event)
{
    switch (event)
    {
    case LV_EVENT_PRESSED:
        sendCommandPowerOn();
        break;
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

    lv_obj_set_event_cb(buttonSendPowerOn, my_event_poweron);
    lv_obj_set_pos(buttonSendPowerOn, 100, 90);
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
