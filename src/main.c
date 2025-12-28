#include "FreeRTOSConfig.h"
#include "esp_task.h"
#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"

// #include "freertos/task.h"
// #include "freertos/timers.h"
// #include <stdlib.h>

#define GPIO_OUT_REG (uint32_t*)0x3FF44004

uint32_t* const output_reg = GPIO_OUT_REG;

static TaskHandle_t green_led_handle;
static TaskHandle_t yellow_led_handle;
static TaskHandle_t red_led_handle;

static BaseType_t status_green_led;
static BaseType_t status_yellow_led;
static BaseType_t status_red_led;

static void toggle_green(void*);
static void toggle_yellow(void*);
static void toggle_red(void*);

static void toggle_green(void*)
{
    TickType_t green_tick = xTaskGetTickCount();
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x02000000;
        xTaskDelayUntil(&green_tick, pdMS_TO_TICKS(1000));
    }
}

static void toggle_yellow(void*)
{
    TickType_t yellow_tick = xTaskGetTickCount();
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x04000000;
        xTaskDelayUntil(&yellow_tick, pdMS_TO_TICKS(800));
    }
}

static void toggle_red(void*)
{
    TickType_t red_tick = xTaskGetTickCount();
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x08000000;
        xTaskDelayUntil(&red_tick, pdMS_TO_TICKS(400));
    }
}

void app_main()
{
    status_green_led = xTaskCreate(toggle_green, "Green LED Task", 1024, NULL, 1, &green_led_handle);
    status_yellow_led = xTaskCreate(toggle_yellow, "Yellow LED Task", 1024, NULL, 1, &yellow_led_handle);
    status_red_led = xTaskCreate(toggle_red, "Red LED Task", 1024, NULL, 1, &red_led_handle);
}