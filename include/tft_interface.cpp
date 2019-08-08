#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <lvgl.h>
#include <Ticker.h>

#define LVGL_TICK_PERIOD 20

Ticker tick;               /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

uint16_t touchpad_y = 0, touchpad_x = 0;
bool my_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void lv_tick_handler(void);
void lv_ex_win_1(void);

void ftoa(float f, char *str, uint8_t precision);

boolean pressTouch = false;
void TaskTft(void *pvParameters);
void TaskTftHandler(void *pvParameters);

void InitGraphics()
{
    lv_init();
    tft.begin();        /* TFT init */
    tft.setRotation(1); /* Landscape orientation */

    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the touch pad*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_input_read;
    lv_indev_drv_register(&indev_drv);

    lv_theme_t *th = lv_theme_night_init(210, &lv_font_roboto_16); //Set a HUE value and a Font for the Night Theme
    lv_theme_set_current(th);

    /*Initialize the graphics library's tick*/
    tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
    xTaskCreate(TaskTftHandler, "task tft handler", 8000, NULL, 2, NULL);
}

void TaskTftHandler(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        lv_task_handler(); /* let the GUI do its work */
        vTaskDelay(5);
    }
}

bool my_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (tft.getTouch(&touchpad_x, &touchpad_y, 600))
    {
        data->state = LV_INDEV_STATE_PR;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = touchpad_x;
    data->point.y = touchpad_y;
    return false; /*No buffering now so no more data read*/
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint16_t c;

    tft.startWrite();                                                                            /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
    for (int y = area->y1; y <= area->y2; y++)
    {
        for (int x = area->x1; x <= area->x2; x++)
        {
            c = color_p->full;
            tft.writeColor(c, 1);
            color_p++;
        }
    }
    tft.endWrite();            /* terminate TFT transaction */
    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{
    lv_tick_inc(LVGL_TICK_PERIOD);
}
