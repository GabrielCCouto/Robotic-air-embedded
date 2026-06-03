#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "motor.h"
#include "sensors.h"
#include "navigation.h"

// ============================================================
// FreeRTOS task configuration
// TODO: adjust priorities and stack sizes based on real measurements
// ============================================================
#define MOTOR_STACK_SIZE    2048
#define SENSOR_STACK_SIZE   2048
#define NAV_STACK_SIZE      4096

#define MOTOR_PRIORITY      3
#define SENSOR_PRIORITY     3
#define NAV_PRIORITY        2

#define MOTOR_QUEUE_LEN     5
#define SENSOR_QUEUE_LEN    5

void app_main(void)
{
    // Create FIFO queues for inter-task communication
    motor_cmd_queue    = xQueueCreate(MOTOR_QUEUE_LEN, sizeof(motor_cmd_t));
    sensor_event_queue = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(sensor_event_t));

    // Initialize peripherals
    motor_init();
    sensors_init();

    // Create FreeRTOS tasks
    // - Motor: high priority, executes PWM commands
    // - Sensor: high priority, polls sensors periodically
    // - Navigation: medium priority, state machine
    xTaskCreate(motor_task,      "motor",  MOTOR_STACK_SIZE,  NULL, MOTOR_PRIORITY,  NULL);
    xTaskCreate(sensor_task,     "sensor", SENSOR_STACK_SIZE, NULL, SENSOR_PRIORITY, NULL);
    xTaskCreate(navigation_task, "nav",    NAV_STACK_SIZE,    NULL, NAV_PRIORITY,    NULL);

    // app_main no longer needed
    vTaskDelete(NULL);
}
