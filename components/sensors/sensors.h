#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_adc/adc_oneshot.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLIFF_FRONT_CHANNEL   ADC_CHANNEL_8
#define CLIFF_ADC_UNIT        ADC_UNIT_1

#define CLIFF_THRESHOLD_RAW   1861
#define CLIFF_POLL_PERIOD_MS  10

#define SENSOR_TASK_STACK     4096
#define SENSOR_TASK_PRIORITY  4

void sensors_init(void);
void sensor_task(void *arg);

#ifdef __cplusplus
}
#endif
