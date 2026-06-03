#include "motor.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "motor";

// ============================================================
// Timing
// ============================================================
#define MOTOR_PERIOD_MS         20   // period of motor task (ramp update)
#define MOTOR_RAMP_STEP          5   // speed increment per cycle (0-100 scale)
#define MOTOR_SPEED_MAX        100

// ============================================================
// PWM (LEDC)
// ============================================================
#define LEDC_FREQ_HZ          5000
#define LEDC_TIMER            LEDC_TIMER_0
#define LEDC_MODE             LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES         LEDC_TIMER_8_BIT
#define LEDC_MAX_DUTY        255

#define LEDC_CH_WHEEL_L       LEDC_CHANNEL_0
#define LEDC_CH_WHEEL_R       LEDC_CHANNEL_1
#define LEDC_CH_SWEEP         LEDC_CHANNEL_2

// ============================================================
// GPIO — TODO: set pins according to physical assembly
// ============================================================
// L298N H-bridge — left motor (wheels)
#define MOTOR_L_IN1           GPIO_NUM_4
#define MOTOR_L_IN2           GPIO_NUM_5
#define MOTOR_L_PWM_ENA       GPIO_NUM_1

// L298N H-bridge — right motor (wheels)
#define MOTOR_R_IN3           GPIO_NUM_6
#define MOTOR_R_IN4           GPIO_NUM_7
#define MOTOR_R_PWM_ENB       GPIO_NUM_2

// Sweeper (brushes) — 1 shared PWM
#define MOTOR_SWEEP_PWM       GPIO_NUM_3

QueueHandle_t motor_cmd_queue = NULL;

static int current_left_speed  = 0;
static int current_right_speed = 0;
static int target_left_speed   = 0;
static int target_right_speed  = 0;

// ------------------------------------------------------------
static void set_direction(motor_direction_t dir, gpio_num_t in1, gpio_num_t in2)
{
    switch (dir) {
    case MOTOR_FORWARD:
        gpio_set_level(in1, 1);
        gpio_set_level(in2, 0);
        break;
    case MOTOR_BACKWARD:
        gpio_set_level(in1, 0);
        gpio_set_level(in2, 1);
        break;
    case MOTOR_STOP:
    default:
        gpio_set_level(in1, 0);
        gpio_set_level(in2, 0);
        break;
    }
}

// ------------------------------------------------------------
static int ramp_apply(int current, int target)
{
    if (current < target) {
        current += MOTOR_RAMP_STEP;
        if (current > target) current = target;
    } else if (current > target) {
        current -= MOTOR_RAMP_STEP;
        if (current < target) current = target;
    }
    return current;
}

// ------------------------------------------------------------
void motor_init(void)
{
    // Direction GPIOs
    gpio_set_direction(MOTOR_L_IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_L_IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_R_IN3, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_R_IN4, GPIO_MODE_OUTPUT);

    // LEDC timer
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = LEDC_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);

    // LEDC channels
    ledc_channel_config_t ch_cfg = {
        .speed_mode = LEDC_MODE,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
        .flags      = {0},
    };

    ch_cfg.gpio_num = MOTOR_L_PWM_ENA;
    ch_cfg.channel  = LEDC_CH_WHEEL_L;
    ledc_channel_config(&ch_cfg);

    ch_cfg.gpio_num = MOTOR_R_PWM_ENB;
    ch_cfg.channel  = LEDC_CH_WHEEL_R;
    ledc_channel_config(&ch_cfg);

    ch_cfg.gpio_num = MOTOR_SWEEP_PWM;
    ch_cfg.channel  = LEDC_CH_SWEEP;
    ledc_channel_config(&ch_cfg);

    ESP_LOGI(TAG, "Motor component initialized");
}

// ------------------------------------------------------------
void motor_task(void *pvParameters)
{
    motor_cmd_t cmd;
    TickType_t last_wake = xTaskGetTickCount();

    while (1) {
        // Check for new command (non-blocking)
        if (xQueueReceive(motor_cmd_queue, &cmd, 0) == pdTRUE) {
            target_left_speed  = (cmd.left_dir  != MOTOR_STOP) ? cmd.left_speed  : 0;
            target_right_speed = (cmd.right_dir != MOTOR_STOP) ? cmd.right_speed : 0;

            set_direction(cmd.left_dir,  MOTOR_L_IN1, MOTOR_L_IN2);
            set_direction(cmd.right_dir, MOTOR_R_IN3, MOTOR_R_IN4);

            int sweep_duty = cmd.sweeper_on ? LEDC_MAX_DUTY : 0;
            ledc_set_duty(LEDC_MODE, LEDC_CH_SWEEP, sweep_duty);
            ledc_update_duty(LEDC_MODE, LEDC_CH_SWEEP);

            ESP_LOGD(TAG, "New cmd: L=%d/%s R=%d/%s sweep=%d",
                     cmd.left_speed,
                     cmd.left_dir == MOTOR_FORWARD ? "F" :
                     cmd.left_dir == MOTOR_BACKWARD ? "B" : "S",
                     cmd.right_speed,
                     cmd.right_dir == MOTOR_FORWARD ? "F" :
                     cmd.right_dir == MOTOR_BACKWARD ? "B" : "S",
                     cmd.sweeper_on);
        }

        // Ramp toward target
        current_left_speed  = ramp_apply(current_left_speed,  target_left_speed);
        current_right_speed = ramp_apply(current_right_speed, target_right_speed);

        int left_duty  = (current_left_speed  * LEDC_MAX_DUTY) / MOTOR_SPEED_MAX;
        int right_duty = (current_right_speed * LEDC_MAX_DUTY) / MOTOR_SPEED_MAX;

        ledc_set_duty(LEDC_MODE, LEDC_CH_WHEEL_L, left_duty);
        ledc_update_duty(LEDC_MODE, LEDC_CH_WHEEL_L);
        ledc_set_duty(LEDC_MODE, LEDC_CH_WHEEL_R, right_duty);
        ledc_update_duty(LEDC_MODE, LEDC_CH_WHEEL_R);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(MOTOR_PERIOD_MS));
    }
}
