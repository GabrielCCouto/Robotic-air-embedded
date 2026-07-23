#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "motor.h"

static const char *TAG = "main";

static void test_motor_sequence(void *arg)
{
    motor_cmd_t cmd;

    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Starting motor test sequence");

    cmd = MOTOR_CMD_FORWARD;
    xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(2000));

    cmd = MOTOR_CMD_STOP;
    xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(1000));

    cmd = MOTOR_CMD_BACKWARD;
    xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(2000));

    cmd = MOTOR_CMD_STOP;
    xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);

    ESP_LOGI(TAG, "Test sequence completed");
    vTaskDelete(NULL);
}

void app_main(void)
{
    motor_init();

    xTaskCreate(motor_task, "motor", MOTOR_STACK_SIZE, NULL, MOTOR_TASK_PRIORITY, NULL);
    xTaskCreate(test_motor_sequence, "test_motor", 4096, NULL, 2, NULL);

    vTaskDelete(NULL);
}
