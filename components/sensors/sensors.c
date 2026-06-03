#include "sensors.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "sensors";

// ============================================================
// Timing
// ============================================================
#define SENSOR_PERIOD_MS        50   // IR sensor polling interval

// ============================================================
// GPIO — TODO: set pins according to physical assembly
// ============================================================
#define SENSOR_IR_FRONT         GPIO_NUM_10
#define SENSOR_IR_LEFT          GPIO_NUM_11
#define SENSOR_IR_RIGHT         GPIO_NUM_12

// Collision sensor (limit switch) — falling edge
#define SENSOR_COLLISION        GPIO_NUM_13

QueueHandle_t sensor_event_queue = NULL;

static volatile bool collision_flag = false;

// ------------------------------------------------------------
static void IRAM_ATTR collision_isr(void *arg)
{
    collision_flag = true;
}

// ------------------------------------------------------------
void sensors_init(void)
{
    // Configure IR sensors as inputs with pull-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SENSOR_IR_FRONT) |
                        (1ULL << SENSOR_IR_LEFT) |
                        (1ULL << SENSOR_IR_RIGHT),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Configure collision sensor with interrupt
    io_conf.pin_bit_mask = (1ULL << SENSOR_COLLISION);
    io_conf.intr_type    = GPIO_INTR_NEGEDGE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(SENSOR_COLLISION, collision_isr, NULL);

    ESP_LOGI(TAG, "Sensors component initialized");
}

// ------------------------------------------------------------
void sensor_task(void *pvParameters)
{
    sensor_event_t evt;

    while (1) {
        // TODO: verify IR active-low vs active-high on hardware
        evt.ir_front  = !gpio_get_level(SENSOR_IR_FRONT);
        evt.ir_left   = !gpio_get_level(SENSOR_IR_LEFT);
        evt.ir_right  = !gpio_get_level(SENSOR_IR_RIGHT);

        // Consume collision ISR flag
        evt.collision = collision_flag;
        if (collision_flag) {
            collision_flag = false;
            ESP_LOGI(TAG, "Collision detected");
        }

        xQueueSend(sensor_event_queue, &evt, pdMS_TO_TICKS(10));

        ESP_LOGD(TAG, "IR F:%d L:%d R:%d Coll:%d",
                 evt.ir_front, evt.ir_left, evt.ir_right, evt.collision);

        vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}
