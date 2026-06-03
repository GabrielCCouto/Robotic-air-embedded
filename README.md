# Robotic Air Embedded

A robotic vacuum cleaner built with **ESP32-S3** and **ESP-IDF v6.0.1**, developed as a hands-on project for an **Embedded Real-Time Systems**.

## Concepts explored

- **Real-Time Scheduling** — preemptive multitasking with FreeRTOS tasks at different priority levels
- **Inter-Task Communication** — message passing via FreeRTOS queues (FIFO)
- **Interrupt Handling** — GPIO interrupt (bumper collision) with deferred processing to a task
- **Resource Synchronization** — mutexes and semaphores for shared state protection (WIP)
- **Memory Management** — conscious stack sizing for each task on ESP32-S3's 512KB SRAM
- **Acceleration Ramping** — periodic task with `vTaskDelayUntil` for smooth motor control

## Hardware

- ESP32-S3
- 2x DC drive motors
- 2x sweep motors (shared PWM)
- 3x IR sensors (front, left, right)
- 1x bumper collision sensor (limit switch with GPIO interrupt)

## Architecture

```
sensor_task (50ms polling)  ──queue──►  navigation_task (event-driven)
                                            │
                                            └──queue──►  motor_task (20ms periodic)
```

Each subsystem lives in a separate ESP-IDF component under `components/`.

## Build & Flash

```bash
idf.py build
idf.py flash
idf.py monitor
idf.py menuconfig
```

## Project status

Early development — most parameters (GPIO pinout, speeds, timing) are placeholders and will be calibrated as hardware is assembled.
