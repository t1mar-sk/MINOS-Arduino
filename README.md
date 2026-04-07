# MINOS-Arduino (Minimal Operating System)

## Overview
MINOS is a priority-based cooperative multitasking kernel designed for the ATmega328P microcontroller. The system implements low-level memory management, inter-process communication (IPC), and execution monitoring to ensure stability in embedded environments.

## Core Architecture

### Memory Management (Heap Segmentation)
The kernel manages memory through two isolated static pools to prevent fragmentation and protect system structures:
* **OS Heap (256 Bytes):** Reserved for kernel-level data, including the task table and system buffers.
* **App Heap (128 Bytes):** Allocated for user-space applications and temporary data processing.

Memory allocation is handled by `minos_malloc(size_t size, bool is_system)`, which manages pointers for both segments independently.

### Task Scheduling
The scheduler utilizes a priority-based cooperative model. Tasks are registered dynamically into the OS Heap via `add_task()`.
* **Selection Logic:** The kernel scans the task table and executes the task with the highest priority (lowest numerical value) that has met its timing requirements.
* **Dynamic Registration:** Tasks are not hardcoded but allocated during the initialization phase or runtime.

### Safety and Monitoring
* **Execution Guard:** Each task has a defined `maxExecTime`. If a task's execution duration exceeds this limit significantly (over 200%), the kernel invokes an emergency stop, disabling the task to prevent system hang.
* **Watchdog Integration:** Real-time monitoring of task performance and system uptime.

## Technical Specifications
* **Target Hardware:** ATmega328P (Arduino Uno/Nano)
* **Language:** C++ (Embedded)
* **Scheduling:** Cooperative Priority
* **IPC:** Mailbox-based messaging

## System Interface (CLI)
The kernel provides a serial interface (9600 baud) for real-time management:
* `ps` - Displays the process table (ID, Name, Status, Priority) and available SRAM.
* `sys_st` - Verifies the current operational status of the kernel.
* `k[ID]` - Terminates (disables) a specific task by its identifier.
* `s[ID]` - Restarts (enables) a previously disabled task.

## Implementation Details
The current version is implemented as a single-file core (`.ino`) to maintain a minimal footprint while ensuring direct register access and efficient memory utilization.
