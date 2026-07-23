# Timing Constraints

RTOS task scheduling and timing analysis for the robotic vacuum project.

## Task Definitions

| Task | Priority | Period | WCET | Deadline | Stack | Type |
|------|----------|--------|------|----------|-------|------|
| `sensor_task` | **4** | Event-driven (ISR) | ~50us | < 1ms | 4096 | Event-driven |
| `motor_task` | 1 | 20ms (ramp update) | ~100us | 50ms | 2048 | Periodic |

- **WCET**: Worst-Case Execution Time (estimated)
- **Deadline**: Maximum acceptable response time
- **Priority**: FreeRTOS numerical priority (higher = more important)

## Inter-Task Communication

### Queues

| Queue | Item Size | Depth | Producer | Consumer |
|-------|-----------|-------|----------|----------|
| `motor_cmd_queue` | sizeof(motor_cmd_t) | 5 | sensor_task | motor_task |

### Event Groups

| Event Group | Bits | Producer | Consumer |
|-------------|------|----------|----------|
| `cliff_event_group` | CLIFF_LEFT_BIT, CLIFF_RIGHT_BIT, CLIFF_FRONT_BIT | ISR (cliff_isr_handler) | sensor_task |

## Scheduling Analysis

### CPU Utilization (estimated)

```
sensor_task:      ~50us per event (cliff detection)
motor_task:       ~100us / 20ms  = 0.5%
-----------------------------------------
Total estimated CPU usage: < 1.0%
```

### Response Time Analysis

**Cliff Detection Response:**
- GPIO interrupt latency: < 10us
- ISR execution: < 5us
- Event group set: < 5us
- Task switch: < 1 tick (1ms default)
- **Total: < 2ms** (well within safety requirements)

## Motor Task Specification

- **Type**: Periodic (20ms ramp update cycle)
- **Behavior**: Starts in FORWARD state by default
- **Commands**: Receives motor_cmd_t with left/right speeds and directions
- **Ramp**: Gradual speed changes (MOTOR_RAMP_STEP = 5 per cycle)

## Sensor Task Specification

- **Type**: Event-driven (waits on cliff_event_group)
- **Trigger**: GPIO ISR on any edge (cliff detection)
- **Action**: Sends MOTOR_STOP to motor_cmd_queue
- **Priority**: 4 (highest) - safety critical

## GPIO Allocation

### Motor (L298N H-Bridge)

| Signal | GPIO | Function |
|--------|------|----------|
| MOTOR_L_IN1 | 4 | Left motor direction |
| MOTOR_L_IN2 | 5 | Left motor direction |
| MOTOR_L_PWM_ENA | 1 | Left motor PWM |
| MOTOR_R_IN3 | 6 | Right motor direction |
| MOTOR_R_IN4 | 7 | Right motor direction |
| MOTOR_R_PWM_ENB | 2 | Right motor PWM |
| MOTOR_SWEEP_PWM | 3 | Sweeper brush PWM |

### Cliff Sensors (IR)

| Signal | GPIO | Function |
|--------|------|----------|
| CLIFF_LEFT | 8 | Left cliff detection |
| CLIFF_RIGHT | 9 | Right cliff detection |
| CLIFF_FRONT | 10 | Front cliff detection |

## Future Tasks (TBD)

| Task | Priority | Period | Description |
|------|----------|--------|-------------|
| `navigation_task` | 2 | 100ms | State machine, obstacle avoidance |
| `battery_task` | 1 | 5s | Monitor battery voltage |
| `scheduler_task` | 5 | 1ms | Custom academic scheduler |
