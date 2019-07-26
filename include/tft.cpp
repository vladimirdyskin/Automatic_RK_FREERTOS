#include "globalVar.h"
#include "tft_callibration.cpp"
#include "tft_interface.cpp"
#include "Arduino.h"


enum but
{
    butCloseMain,
    butCloseRect
};

const int countButtons = 10;
lv_obj_t* buttons[countButtons];
lv_obj_t *win; 
lv_obj_t *txtTemp[4];
lv_obj_t *screenMain, *screenRect;
extern float tempValue[5];
extern void publishValueMqtt();

void lv_ex_win_main(void);
void lv_ex_win_1_peregon(void);
void TaskUpdateValue(void *pvParameters);


void tftSetup()
{
    InitGraphics();

    lv_ex_win_main();
    lv_ex_win_1_peregon();
    vTaskDelay(1000);
    lv_scr_load(screenMain);

    xTaskCreate( TaskUpdateValue, "task update value", 8000, NULL, 3, NULL);
}



void TaskUpdateValue(void *pvParameters)
{
    (void) pvParameters;

    static float pLcdT = 0;
    for(;;)
    {
        if((pLcdT != tempValue[0]) && (txtTemp[0] != 0) /* && val != -127 && val != 85*/)
        {
            char buf[40];
            snprintf(buf, 40,"Temp1: %.2f", tempValue[0]);
            lv_label_set_text(txtTemp[0], buf);
            publishValueMqtt();
        }
        pLcdT = tempValue[0];
        vTaskDelay(1000);
    }
}


static void my_event_cb(lv_obj_t * obj, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_PRESSED:
                if(obj == buttons[but::butCloseMain])
                {
                   lv_scr_load(screenRect);
                   Serial.println("Press close win\n");            
                }
                if(obj == buttons[but::butCloseRect])
                {
                   lv_scr_load(screenMain);
                   Serial.println("Press close rect\n");            
                }
            break;

        case LV_EVENT_SHORT_CLICKED:
            break;

        case LV_EVENT_CLICKED:
            Serial.println("Clicked\n");
            break;

        case LV_EVENT_LONG_PRESSED:
            Serial.println("Long press\n");
            break;

        case LV_EVENT_LONG_PRESSED_REPEAT:
            Serial.println("Long press repeat\n");
            break;

        case LV_EVENT_RELEASED:
            Serial.println("Released\n");
            break;
    }      /*Etc.*/
}

void lv_ex_win_main(void)
{
    screenMain = lv_obj_create(NULL, NULL);
    /*Create a window*/
    win = lv_win_create(screenMain, NULL);
    lv_win_set_title(win, "Temp sensor"); 
    /*Add control button to the header*/
    lv_obj_t * close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE);           /*Add close button and use built-in close action*/
    buttons[but::butCloseRect] = close_btn;
    lv_obj_set_event_cb(close_btn, my_event_cb);
    lv_win_add_btn(win, LV_SYMBOL_SETTINGS);        /*Add a setup button*/

    /*Set the title*/
    /*Add some dummy content*/
    txtTemp[0] = lv_label_create(win, NULL);
    lv_label_set_text(txtTemp[0], " ");
}

void lv_ex_win_1_peregon(void)
{
    screenRect = lv_obj_create(NULL, NULL);
    /*Create a window*/
    lv_obj_t * win = lv_win_create(screenRect, NULL);
    lv_win_set_title(win, "1 peregon");                        /*Set the title*/


    /*Add control button to the header*/
    lv_obj_t * close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE);           /*Add close button and use built-in close action*/
    buttons[but::butCloseRect] = close_btn;
    lv_obj_set_event_cb(close_btn, my_event_cb);
    lv_win_add_btn(win, LV_SYMBOL_SETTINGS);        /*Add a setup button*/

    /*Add content to the tabs*/
    lv_obj_t * label = lv_label_create(win, NULL);
    lv_label_set_text(label, "\nThis the first win\n\n"
                             "1 Peregon\n"
                             "scrollable.");
}

