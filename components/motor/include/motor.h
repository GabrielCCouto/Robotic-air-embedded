#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define MOTOR_IN1    GPIO_NUM_10
#define MOTOR_IN2    GPIO_NUM_11
#define MOTOR_IN3    GPIO_NUM_12
#define MOTOR_IN4    GPIO_NUM_13

#define MOTOR_STACK_SIZE    2048
#define MOTOR_TASK_PRIORITY 1
#define MOTOR_QUEUE_LEN     5

typedef enum {
    MOTOR_CMD_FORWARD,
    MOTOR_CMD_BACKWARD,
    MOTOR_CMD_STOP
} motor_cmd_t;

extern QueueHandle_t motor_cmd_queue;

void motor_init(void);
void motor_task(void *arg);
