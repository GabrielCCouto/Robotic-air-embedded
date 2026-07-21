# Timing Constraints

RTOS task scheduling and timing analysis for the robotic vacuum project.

## Task Definitions

| Task | Priority | Period | WCET | Deadline | Stack | Type |
|------|----------|--------|------|----------|-------|------|
| `sensor_task` | 3 | 50ms | ~200us | 50ms | 2048 | Periodic |
| `navigation_task` | 2 | 100ms | ~500us | 100ms | 4096 | Periodic |
| `motor_task` | 1 | N/A | ~100us | 50ms | 2048 | Event-driven |

- **WCET**: Worst-Case Execution Time (estimated)
- **Deadline**: Maximum acceptable response time
- **Priority**: FreeRTOS numerical priority (higher = more important)

## Queue Definitions

| Queue | Item Size | Depth | Producer | Consumer |
|-------|-----------|-------|----------|----------|
| `motor_cmd_queue` | sizeof(motor_cmd_t) | 5 | navigation_task | motor_task |
| `sensor_event_queue` | sizeof(sensor_event_t) | 5 | sensor_task | navigation_task |

## Scheduling Analysis

### CPU Utilization (estimated)

```
sensor_task:      200us / 50ms   = 0.4%
navigation_task:  500us / 100ms  = 0.5%
motor_task:       100us / event  = ~0.1%
-----------------------------------------
Total estimated CPU usage: ~1.0%
```

### Priority Inversion Protection

- All tasks use same shared resources (none currently)
- Mutexes will be added if shared peripherals are introduced

## Motor Task Specification

- **Type**: Event-driven (waits on `motor_cmd_queue`)
- **Commands**: FORWARD, BACKWARD, STOP
- **Response time**: < 1ms (queue receive + GPIO set)
- **Blocking behavior**: Blocks on `xQueueReceive()` until command arrives

## Future Tasks (TBD)

| Task | Priority | Period | Description |
|------|----------|--------|-------------|
| `battery_task` | 1 | 5s | Monitor battery voltage |
| `scheduler_task` | 4 | 1ms | Custom academic scheduler |
