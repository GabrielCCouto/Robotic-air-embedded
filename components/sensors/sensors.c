#include "sensors.h"
#include "motor.h"
#include "esp_log.h"

static const char *TAG = "sensors";

static adc_oneshot_unit_handle_t adc_handle = NULL;

static void cliff_adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = CLIFF_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_0,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, CLIFF_FRONT_CHANNEL, &chan_cfg));

    ESP_LOGI(TAG, "Cliff ADC initialized (FRONT=CH%d)", CLIFF_FRONT_CHANNEL);
}

void sensors_init(void)
{
    cliff_adc_init();
}

void sensor_task(void *arg)
{
    int raw_front;
    motor_cmd_t cmd;

    ESP_LOGI(TAG, "Sensor task started (priority %d)", SENSOR_TASK_PRIORITY);

    while (1) {
        adc_oneshot_read(adc_handle, CLIFF_FRONT_CHANNEL, &raw_front);

        if (raw_front > CLIFF_THRESHOLD_RAW) {
            ESP_LOGW(TAG, "CLIFF detected: FRONT (raw=%d)", raw_front);
            cmd = MOTOR_CMD_STOP;
            xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);
        } else {
            cmd = MOTOR_CMD_FORWARD;
            xQueueSend(motor_cmd_queue, &cmd, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(CLIFF_POLL_PERIOD_MS));
    }
}
