#include "navigation.h"
#include "esp_log.h"

static const char *TAG = "navigation";

// ============================================================
// Navigation parameters — TODO: calibrate with real hardware
// ============================================================
#define NAV_SPEED_FORWARD       60
#define NAV_SPEED_TURN          40
#define NAV_SPEED_REVERSE       50

// Duration in cycles (each cycle ≅ motor task period = 20ms)
#define NAV_REVERSE_CYCLES      10    // reverse ~200ms
#define NAV_ESCAPE_TURN_CYCLES  20    // turn ~400ms
#define NAV_SIDE_TURN_CYCLES    10    // side correction ~200ms

// ------------------------------------------------------------
void navigation_task(void *pvParameters)
{
    sensor_event_t evt;
    nav_state_t state = NAV_FORWARD;
    motor_cmd_t cmd;
    int maneuver_counter = 0;

    while (1) {
        // Wait for sensor data (blocking)
        xQueueReceive(sensor_event_queue, &evt, portMAX_DELAY);

        switch (state) {

        // ========================================================
        case NAV_FORWARD:
            if (evt.collision) {
                state = NAV_REVERSE;
                maneuver_counter = NAV_REVERSE_CYCLES;
                ESP_LOGI(TAG, "Collision -> REVERSE");
                break;
            }

            if (!evt.ir_front) {
                // TODO: obstacle avoidance — for now just turn
                state = NAV_TURN_RIGHT;
                maneuver_counter = NAV_SIDE_TURN_CYCLES;
                ESP_LOGI(TAG, "Front obstacle -> TURN");
                break;
            }

            // Move forward
            cmd.left_dir   = MOTOR_FORWARD;
            cmd.right_dir  = MOTOR_FORWARD;
            cmd.left_speed = NAV_SPEED_FORWARD;
            cmd.right_speed = NAV_SPEED_FORWARD;
            cmd.sweeper_on = true;
            xQueueSend(motor_cmd_queue, &cmd, pdMS_TO_TICKS(10));
            break;

        // ========================================================
        case NAV_REVERSE:
            cmd.left_dir   = MOTOR_BACKWARD;
            cmd.right_dir  = MOTOR_BACKWARD;
            cmd.left_speed = NAV_SPEED_REVERSE;
            cmd.right_speed = NAV_SPEED_REVERSE;
            cmd.sweeper_on = true;
            xQueueSend(motor_cmd_queue, &cmd, pdMS_TO_TICKS(10));

            if (--maneuver_counter <= 0) {
                state = NAV_ESCAPE;
                maneuver_counter = NAV_ESCAPE_TURN_CYCLES;
                ESP_LOGI(TAG, "REVERSE -> ESCAPE");
            }
            break;

        // ========================================================
        case NAV_ESCAPE:
            cmd.left_dir   = MOTOR_FORWARD;
            cmd.right_dir  = MOTOR_BACKWARD;
            cmd.left_speed = NAV_SPEED_TURN;
            cmd.right_speed = NAV_SPEED_TURN;
            cmd.sweeper_on = true;
            xQueueSend(motor_cmd_queue, &cmd, pdMS_TO_TICKS(10));

            if (--maneuver_counter <= 0) {
                state = NAV_FORWARD;
                ESP_LOGI(TAG, "ESCAPE -> FORWARD");
            }
            break;

        // ========================================================
        case NAV_TURN_LEFT:
            cmd.left_dir   = MOTOR_BACKWARD;
            cmd.right_dir  = MOTOR_FORWARD;
            cmd.left_speed = NAV_SPEED_TURN;
            cmd.right_speed = NAV_SPEED_TURN;
            cmd.sweeper_on = true;
            xQueueSend(motor_cmd_queue, &cmd, pdMS_TO_TICKS(10));

            if (--maneuver_counter <= 0) {
                state = NAV_FORWARD;
                ESP_LOGI(TAG, "TURN_LEFT -> FORWARD");
            }
            break;

        // ========================================================
        case NAV_TURN_RIGHT:
            cmd.left_dir   = MOTOR_FORWARD;
            cmd.right_dir  = MOTOR_BACKWARD;
            cmd.left_speed = NAV_SPEED_TURN;
            cmd.right_speed = NAV_SPEED_TURN;
            cmd.sweeper_on = true;
            xQueueSend(motor_cmd_queue, &cmd, pdMS_TO_TICKS(10));

            if (--maneuver_counter <= 0) {
                state = NAV_FORWARD;
                ESP_LOGI(TAG, "TURN_RIGHT -> FORWARD");
            }
            break;

        // ========================================================
        case NAV_STOP:
        default:
            cmd.left_dir   = MOTOR_STOP;
            cmd.right_dir  = MOTOR_STOP;
            cmd.left_speed = 0;
            cmd.right_speed = 0;
            cmd.sweeper_on = false;
            xQueueSend(motor_cmd_queue, &cmd, pdMS_TO_TICKS(10));
            break;
        }
    }
}
