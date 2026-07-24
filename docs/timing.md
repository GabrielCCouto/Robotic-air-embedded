# Timing Constraints

RTOS task scheduling and timing analysis for the robotic vacuum project.

## Task Definitions

| Task | Priority | Period | WCET | Deadline | Stack | Type |
|------|----------|--------|------|----------|-------|------|
| `sensor_task` | **4** | 10ms (polling) | ~30us | 10ms | 4096 | Periodic |
| `motor_task` | 1 | 20ms (ramp update) | ~100us | 50ms | 2048 | Periodic |

- **WCET**: Worst-Case Execution Time (estimated)
- **Deadline**: Maximum acceptable response time
- **Priority**: FreeRTOS numerical priority (higher = more important)

## Inter-Task Communication

### Queues

| Queue | Item Size | Depth | Producer | Consumer |
|-------|-----------|-------|----------|----------|
| `motor_cmd_queue` | sizeof(motor_cmd_t) | 5 | sensor_task | motor_task |

## Scheduling Analysis

### CPU Utilization (estimated)

```
sensor_task:      ~30us / 10ms  = 0.3%
motor_task:       ~100us / 20ms = 0.5%
-----------------------------------------
Total estimated CPU usage: < 1.0%
```

### Response Time Analysis

**Cliff Detection Response:**
- ADC read: ~10us per channel
- Threshold comparison: < 1us
- Queue send: ~5us
- Task switch: < 1 tick (1ms default)
- **Total: < 12ms** (within 10ms polling period + context switch)

## Motor Task Specification

- **Type**: Periodic (20ms ramp update cycle)
- **Behavior**: Starts in FORWARD state by default
- **Commands**: Receives motor_cmd_t (enum: FORWARD, BACKWARD, STOP)
- **Ramp**: Gradual speed changes (MOTOR_RAMP_STEP = 5 per cycle)

## Sensor Task Specification

- **Type**: Periodic polling (10ms cycle)
- **ADC**: esp_adc oneshot, ADC1, CH8 (GPIO 9)
- **Attenuation**: ADC_ATTEN_DB_0 (0-1.1V range)
- **Threshold**: Raw > 1861 (~500mV) triggers cliff detection
- **Action**: Sends MOTOR_STOP to motor_cmd_queue
- **Priority**: 4 (highest) - safety critical

## GPIO / ADC Allocation

### Motor (L298N H-Bridge — ENA/ENB hardcoded HIGH via jumper)

| Signal | GPIO | Function |
|--------|------|----------|
| MOTOR_IN1 | 10 | Motor A direction |
| MOTOR_IN2 | 11 | Motor A direction |
| MOTOR_IN3 | 12 | Motor B direction |
| MOTOR_IN4 | 13 | Motor B direction |

### Cliff Sensors (IR - Analog)

| Signal | GPIO | ADC Channel | Function |
|--------|------|-------------|----------|
| CLIFF_FRONT | 9 | ADC1 CH8 | Front cliff detection |
| CLIFF_LEFT | 8 | ADC1 CH7 | Left cliff detection (TBD) |
| CLIFF_RIGHT | — | — | Right cliff detection (TBD) |

## Future Tasks (TBD)

| Task | Priority | Period | Description |
|------|----------|--------|-------------|
| `navigation_task` | 2 | 100ms | State machine, obstacle avoidance |
| `battery_task` | 1 | 5s | Monitor battery voltage |
| `scheduler_task` | 5 | 1ms | Custom academic scheduler |
