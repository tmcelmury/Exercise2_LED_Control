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

static TaskHandle_t volatile next_task_handle = NULL;

static BaseType_t status_green_led;
static BaseType_t status_yellow_led;
static BaseType_t status_red_led;

static BaseType_t notify_status;

static void toggle_green(void*);
static void toggle_yellow(void*);
static void toggle_red(void*);
static void button_handler(void* args);

static void button_handler(void* args)
{
    xTaskNotify(next_task_handle, 0, eNoAction);
}

static void toggle_green(void*)
{
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
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_pullup_en(GPIO_NUM_0);
    gpio_pulldown_dis(GPIO_NUM_0);
    gpio_intr_enable(GPIO_NUM_0);
    gpio_set_intr_type(GPIO_NUM_0, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_0, button_handler, (void*) NULL);
    
    status_green_led = xTaskCreate(toggle_green, "Green LED Task", 2048, NULL, 3, &green_led_handle);
    next_task_handle = green_led_handle;
    status_yellow_led = xTaskCreate(toggle_yellow, "Yellow LED Task", 1024, NULL, 2, &yellow_led_handle);
    status_red_led = xTaskCreate(toggle_red, "Red LED Task", 1024, NULL, 1, &red_led_handle);
}