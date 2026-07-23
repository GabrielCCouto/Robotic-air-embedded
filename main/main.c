#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "motor.h"
#include "sensors.h"

static const char *TAG = "main";

#define MOTOR_STACK_SIZE    2048
#define MOTOR_PRIORITY      1
#define MOTOR_QUEUE_LEN     5

#define SENSOR_STACK_SIZE   4096
#define SENSOR_PRIORITY     4

void app_main(void)
{
    ESP_LOGI(TAG, "Robot starting...");

    motor_init();
    sensors_init();

    xTaskCreate(motor_task,  "motor",  MOTOR_STACK_SIZE,  NULL, MOTOR_PRIORITY,  NULL);
    xTaskCreate(sensor_task, "sensor", SENSOR_STACK_SIZE, NULL, SENSOR_PRIORITY, NULL);

    ESP_LOGI(TAG, "Tasks created. Motor default: FORWARD");

    vTaskDelete(NULL);
}
