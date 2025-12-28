#include "FreeRTOSConfig.h"
#include "esp_task.h"
#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"

uint32_t* const output_reg = (uint32_t*)0x3FF44004;
uint32_t* const input_reg = (uint32_t*)0x3FF4403C;

static TaskHandle_t green_led_handle;
static TaskHandle_t yellow_led_handle;
static TaskHandle_t red_led_handle;
static TaskHandle_t button_handle;

static TaskHandle_t volatile next_task_handle = NULL;

static BaseType_t status_green_led;
static BaseType_t status_yellow_led;
static BaseType_t status_red_led;
static BaseType_t status_button;

static void toggle_green(void*);
static void toggle_yellow(void*);
static void toggle_red(void*);
static void button_handler(void*);

static void button_handler(void*)
{
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_pullup_en(GPIO_NUM_0);
    bool previous_button_state = 1;
    bool current_button_state = 1;
    while (true)
    {
        current_button_state = gpio_get_level(GPIO_NUM_0);
        if (previous_button_state == 1 && current_button_state == 0)
        {
            xTaskNotify(next_task_handle, 0, eNoAction);
        }
        previous_button_state = current_button_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void toggle_green(void*)
{
    BaseType_t notify_status;
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x02000000;
        notify_status = xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(1000));
        if (notify_status == pdTRUE)
        {
            next_task_handle = yellow_led_handle;
            gpio_set_level(GPIO_NUM_25, 0);
            vTaskDelete(NULL);
        }
    }
}

static void toggle_yellow(void*)
{
    BaseType_t notify_status;
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x04000000;
        notify_status = xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(800));
        if (notify_status == pdTRUE)
        {
            next_task_handle = red_led_handle;
            gpio_set_level(GPIO_NUM_26, 0);
            vTaskDelete(NULL);
        }
    }
}

static void toggle_red(void*)
{
    BaseType_t notify_status;
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x08000000;
        notify_status = xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(400));
        if (notify_status == pdTRUE)
        {
            gpio_set_level(GPIO_NUM_27, 0);
            vTaskDelete(NULL);
        }
    }
}

void app_main()
{
    status_button = xTaskCreate(button_handler, "Button Handler Task", 1024, NULL, 4, &button_handle);
    status_green_led = xTaskCreate(toggle_green, "Green LED Task", 1024, NULL, 3, &green_led_handle);
    next_task_handle = green_led_handle;
    status_yellow_led = xTaskCreate(toggle_yellow, "Yellow LED Task", 1024, NULL, 2, &yellow_led_handle);
    status_red_led = xTaskCreate(toggle_red, "Red LED Task", 1024, NULL, 1, &red_led_handle);
}