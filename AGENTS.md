# Robotic Air Embedded

Robotic vacuum cleaner — embedded real-time systems course project.

## Platform

- **SoC:** ESP32
- **SDK:** ESP-IDF v6.0.1
- **RTOS:** FreeRTOS (native to ESP-IDF)
- **Languages:** C and C++

## Build & Deploy

```bash
idf.py build
idf.py flash
idf.py monitor
idf.py menuconfig
idf.py build flash monitor   # full cycle shortcut
```

## Structure

```
components/
  motor/         # PWM + direction (wheels, sweeper)
  sensors/       # IR (3x), collision (limit switch w/ ISR)
  navigation/    # state machine
  scheduler/     # TBD: custom scheduler (academic)
  battery/       # TBD: power management
main/
  main.c         # app_main, task and queue init
```

## Code conventions

- `.c` files for procedural code, `.cpp` when using C++ classes.
- `snake_case` for functions and variables.
- `PascalCase` for C++ classes.
- Indentation: 4 spaces.
- Comments in English.

## RTOS / Tasks

- FreeRTOS tasks (motor prio 3, sensor prio 3, navigation prio 2).
- Inter-task communication via **FreeRTOS queues** (FIFO):
  - Collision ISR → volatile flag → sensor task
  - sensor task → `sensor_event_queue` → navigation task
  - navigation task → `motor_cmd_queue` → motor task
- Synchronization via **semaphores / mutexes** (TBD).
- Motor task uses `vTaskDelayUntil` for periodic acceleration ramp.
- Sensor task uses `vTaskDelay` with 50ms polling.

## Debug

- Log via UART serial (`ESP_LOGI`, `ESP_LOGE`, etc.).
- Log level configuration via `menuconfig`.

## Memory

- ESP32 internal heap (520KB SRAM).
- Careful task stack sizing required.
- TBD: allocation strategy (static vs dynamic).

## TBD (to be defined during the course)

- Pinout (GPIOs, peripherals).
- Final robot modules.
- Custom academic scheduler vs plain FreeRTOS.
- Inter-module communication protocol.
