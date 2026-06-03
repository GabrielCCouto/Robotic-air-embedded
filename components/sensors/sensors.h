#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool ir_front;
    bool ir_left;
    bool ir_right;
    bool collision;
} sensor_event_t;

extern QueueHandle_t sensor_event_queue;

void sensors_init(void);
void sensor_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
