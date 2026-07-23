#include "sensors.h"
#include "motor.h"
#include "esp_log.h"

static const char *TAG = "sensors";

EventGroupHandle_t cliff_event_group = NULL;

void IRAM_ATTR cliff_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t pin = (uint32_t)arg;

    switch (pin) {
        case CLIFF_LEFT_PIN:
            xEventGroupSetBitsFromISR(cliff_event_group, CLIFF_LEFT_BIT, &xHigherPriorityTaskWoken);
            break;
        case CLIFF_RIGHT_PIN:
            xEventGroupSetBitsFromISR(cliff_event_group, CLIFF_RIGHT_BIT, &xHigherPriorityTaskWoken);
            break;
        case CLIFF_FRONT_PIN:
            xEventGroupSetBitsFromISR(cliff_event_group, CLIFF_FRONT_BIT, &xHigherPriorityTaskWoken);
            break;
    }

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void cliff_gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CLIFF_LEFT_PIN) |
                        (1ULL << CLIFF_RIGHT_PIN) |
                        (1ULL << CLIFF_FRONT_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(CLIFF_LEFT_PIN,  cliff_isr_handler, (void *)CLIFF_LEFT_PIN);
    gpio_isr_handler_add(CLIFF_RIGHT_PIN, cliff_isr_handler, (void *)CLIFF_RIGHT_PIN);
    gpio_isr_handler_add(CLIFF_FRONT_PIN, cliff_isr_handler, (void *)CLIFF_FRONT_PIN);

    ESP_LOGI(TAG, "Cliff sensors initialized (LEFT=%d RIGHT=%d FRONT=%d)",
             CLIFF_LEFT_PIN, CLIFF_RIGHT_PIN, CLIFF_FRONT_PIN);
}

void sensors_init(void)
{
    cliff_event_group = xEventGroupCreate();
    if (cliff_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create cliff event group");
        return;
    }

    cliff_gpio_init();
}

void sensor_task(void *arg)
{
    EventBits_t bits;
    motor_cmd_t cmd;

    ESP_LOGI(TAG, "Sensor task started (priority %d)", SENSOR_TASK_PRIORITY);

    while (1) {
        bits = xEventGroupWaitBits(cliff_event_group,
                                   CLIFF_ANY_BIT,
                                   pdTRUE,
                                   pdFALSE,
                                   portMAX_DELAY);

        if (bits & CLIFF_LEFT_BIT) {
            ESP_LOGW(TAG, "CLIFF detected: LEFT");
        }
        if (bits & CLIFF_RIGHT_BIT) {
            ESP_LOGW(TAG, "CLIFF detected: RIGHT");
        }
        if (bits & CLIFF_FRONT_BIT) {
            ESP_LOGW(TAG, "CLIFF detected: FRONT");
        }

        ESP_LOGW(TAG, "Emergency STOP - cliff detected!");
        cmd = MOTOR_CMD_STOP;
        xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);
    }
}
