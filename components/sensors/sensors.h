#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLIFF_LEFT_PIN    GPIO_NUM_8
#define CLIFF_RIGHT_PIN   GPIO_NUM_9
#define CLIFF_FRONT_PIN   GPIO_NUM_10

#define CLIFF_LEFT_BIT    (1 << 0)
#define CLIFF_RIGHT_BIT   (1 << 1)
#define CLIFF_FRONT_BIT   (1 << 2)

#define CLIFF_ANY_BIT     (CLIFF_LEFT_BIT | CLIFF_RIGHT_BIT | CLIFF_FRONT_BIT)

#define SENSOR_TASK_STACK    4096
#define SENSOR_TASK_PRIORITY 4

extern EventGroupHandle_t cliff_event_group;

void sensors_init(void);
void sensor_task(void *arg);
void cliff_isr_handler(void *arg);

#ifdef __cplusplus
}
#endif
