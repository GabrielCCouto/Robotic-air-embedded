#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MOTOR_STOP,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
} motor_direction_t;

typedef struct {
    int left_speed;
    int right_speed;
    motor_direction_t left_dir;
    motor_direction_t right_dir;
    bool sweeper_on;
} motor_cmd_t;

extern QueueHandle_t motor_cmd_queue;

void motor_init(void);
void motor_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
