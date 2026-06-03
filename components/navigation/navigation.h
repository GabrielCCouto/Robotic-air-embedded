#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "motor.h"
#include "sensors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NAV_FORWARD,
    NAV_REVERSE,
    NAV_TURN_LEFT,
    NAV_TURN_RIGHT,
    NAV_ESCAPE,
    NAV_STOP,
} nav_state_t;

void navigation_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
