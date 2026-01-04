#include "FreeRTOSConfig.h"
#include "esp_task.h"
#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"

uint32_t* const output_reg = (uint32_t*)0x3FF44004;

static TaskHandle_t yellow_led_handle;
static TaskHandle_t red_led_handle;

static BaseType_t status_yellow_led;
static BaseType_t status_red_led;

static TickType_t yellow_tick;
static TickType_t red_tick;

//static portMUX_TYPE isr_mux;
static bool button_status = false;

static void toggle_yellow(void*);
static void toggle_red(void*);
static void button_handler(void* args);
static void switch_priority();

static void button_handler(void* args)
{
    button_status = true;
}

static void switch_priority()
{
    if (button_status)
    {
        TaskHandle_t current_task_handle, other_task_handle;
        BaseType_t temp_priority;

        current_task_handle = xTaskGetCurrentTaskHandle();
        if (current_task_handle == yellow_led_handle)
        {
            other_task_handle = red_led_handle;
        }
        else
        {
            other_task_handle = yellow_led_handle;
        }
        temp_priority = uxTaskPriorityGet(current_task_handle);
        vTaskPrioritySet(current_task_handle, uxTaskPriorityGet(other_task_handle));
        vTaskPrioritySet(other_task_handle, temp_priority);
        button_status = false;
    }
}

static void toggle_yellow(void*)
{
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x04000000;
        switch_priority();
        xTaskDelayUntil(&yellow_tick, pdMS_TO_TICKS(800));
    }
}

static void toggle_red(void*)
{
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
    while (true)
    {
        (*output_reg) ^= 0x08000000;
        switch_priority();
        xTaskDelayUntil(&red_tick, pdMS_TO_TICKS(400));
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
    
    status_yellow_led = xTaskCreate(toggle_yellow, "Yellow LED Task", 1024, NULL, 3, &yellow_led_handle);
    status_red_led = xTaskCreate(toggle_red, "Red LED Task", 1024, NULL, 2, &red_led_handle);
}