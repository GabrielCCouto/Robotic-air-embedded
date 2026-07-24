#include "motor.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "motor";

QueueHandle_t motor_cmd_queue = NULL;

static void motor_set_stop(void)
{
    gpio_set_level(MOTOR_IN1, 0);
    gpio_set_level(MOTOR_IN2, 0);
    gpio_set_level(MOTOR_IN3, 0);
    gpio_set_level(MOTOR_IN4, 0);
}

static void motor_gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTOR_IN1) |
                        (1ULL << MOTOR_IN2) |
                        (1ULL << MOTOR_IN3) |
                        (1ULL << MOTOR_IN4),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    motor_set_stop();
}

static void motor_set_forward(void)
{
    gpio_set_level(MOTOR_IN1, 1);
    gpio_set_level(MOTOR_IN2, 0);
    gpio_set_level(MOTOR_IN3, 1);
    gpio_set_level(MOTOR_IN4, 0);
}

static void motor_set_backward(void)
{
    gpio_set_level(MOTOR_IN1, 0);
    gpio_set_level(MOTOR_IN2, 1);
    gpio_set_level(MOTOR_IN3, 0);
    gpio_set_level(MOTOR_IN4, 1);
}

void motor_init(void)
{
    motor_gpio_init();

    motor_cmd_queue = xQueueCreate(MOTOR_QUEUE_LEN, sizeof(motor_cmd_t));
    if (motor_cmd_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create motor command queue");
    }

    ESP_LOGI(TAG, "Motor initialized (IN1=%d IN2=%d IN3=%d IN4=%d)",
             MOTOR_IN1, MOTOR_IN2, MOTOR_IN3, MOTOR_IN4);
}

void motor_task(void *arg)
{
    motor_cmd_t cmd;

    ESP_LOGI(TAG, "Motor task started → FORWARD");
    motor_set_forward();

    while (1) {
        if (xQueueReceive(motor_cmd_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            switch (cmd) {
                case MOTOR_CMD_FORWARD:
                    ESP_LOGI(TAG, "Command: FORWARD");
                    motor_set_forward();
                    break;
                case MOTOR_CMD_BACKWARD:
                    ESP_LOGI(TAG, "Command: BACKWARD");
                    motor_set_backward();
                    break;
                case MOTOR_CMD_STOP:
                    ESP_LOGI(TAG, "Command: STOP");
                    motor_set_stop();
                    break;
            }
        }
    }
}
