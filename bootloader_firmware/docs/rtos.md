# SOME NOTES
------------------------

## While Debugging

check this link: https://www.youtube.com/watch?v=_1u7IOnivnM

If the chip is from China, then edit the file stm32f1x.cfg like

https://github.com/arduino/OpenOCD/blob/master/tcl/target/stm32f1x.cfg#L34

this line can be start with 2
or directly make it 0, not to check chip ID at all


## How to Add RTOS

- edit CMakeLists.txt

```
file(GLOB free_rtos_all
     "../../thirdparty/FreeRTOS-Kernel/*.c"
)

../../thirdparty/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c
../../thirdparty/FreeRTOS-Kernel/portable/MemMang/heap_3.c
${free_rtos_all}

../../thirdparty/FreeRTOS-Kernel/include
../../thirdparty/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
```

- edit main.c

```
#include "FreeRTOS.h"
#include "task.h"

void mytask(void *data) {
    (void)data;
    //USER CODE
}

xTaskCreate(mytask, "mytask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

vTaskStartScheduler();
```

- and find a suitable FreeRTOSConfig.h and put it in "inc"
- and remove some functions from it source externed in the freertos config (SVC_Handler-PendSV_Handler-SysTick_Handler)
- the most important thing is system clock will not be systick, use a TIM for that

## RTOS API

### Characteristics of a 'Task'
In brief: A real time application that uses an RTOS can be structured as a set of independent tasks. Each task executes within its own context with no coincidental dependency on other tasks within the system or the RTOS scheduler itself. Only one task within the application can be executing at any point in time and the real time RTOS scheduler is responsible for deciding which task this should be. The RTOS scheduler may therefore repeatedly start and stop each task (swap each task in and out) as the application executes. As a task has no knowledge of the RTOS scheduler activity it is the responsibility of the real time RTOS scheduler to ensure that the processor context (register values, stack contents, etc) when a task is swapped in is exactly that as when the same task was swapped out. To achieve this each task is provided with its own stack.

### Task States
A task can exist in one of the following states:

#### Running
When a task is actually executing it is said to be in the Running state. It is currently utilising the processor. If the processor on which the RTOS is running only has a single core then there can only be one task in the Running state at any given time.

#### Ready
Ready tasks are those that are able to execute (they are not in the Blocked or Suspended state) but are not currently executing because a different task of equal or higher priority is already in the Running state.

#### Blocked
A task is said to be in the Blocked state if it is currently waiting for either a temporal or external event. For example, if a task calls vTaskDelay() it will block (be placed into the Blocked state) until the delay period has expired - a temporal event. Tasks can also block to wait for queue, semaphore, event group, notification or semaphore event. Tasks in the Blocked state normally have a 'timeout' period, after which the task will be timeout, and be unblocked, even if the event the task was waiting for has not occurred.

Tasks in the Blocked state do not use any processing time and cannot be selected to enter the Running state.

#### Suspended
Like tasks that are in the Blocked state, tasks in the Suspended state cannot be selected to enter the Running state, but tasks in the Suspended state do not have a time out. Instead, tasks only enter or exit the Suspended state when explicitly commanded to do so through the vTaskSuspend() and xTaskResume() API calls respectively.

![Valid task state transitions](assets/task_states.JPG)

### Task Priorities
Each task is assigned a priority from 0 to ( configMAX_PRIORITIES - 1 ), where configMAX_PRIORITIES is defined within FreeRTOSConfig.h.
If the port in use implements a port optimised task selection mechanism that uses a 'count leading zeros' type instruction (for task selection in a single instruction) and configUSE_PORT_OPTIMISED_TASK_SELECTION is set to 1 in FreeRTOSConfig.h, then configMAX_PRIORITIES cannot be higher than 32. In all other cases configMAX_PRIORITIES can take any value within reason - but for reasons of RAM usage efficiency should be kept to the minimum value actually necessary.

Low priority numbers denote low priority tasks. The idle task has priority zero (tskIDLE_PRIORITY).

The FreeRTOS scheduler ensures that tasks in the Ready or Running state will always be given processor (CPU) time in preference to tasks of a lower priority that are also in the ready state. In other words, the task placed into the Running state is always the highest priority task that is able to run.

Any number of tasks can share the same priority. If configUSE_TIME_SLICING is not defined, or if configUSE_TIME_SLICING is set to 1, then Ready state tasks of equal priority will share the available processing time using a time sliced round robin scheduling scheme.

### FreeRTOS scheduling

#### The default RTOS scheduling policy (single-core)
By default, FreeRTOS uses a fixed-priority preemptive scheduling policy, with round-robin time-slicing of equal priority tasks:

"Fixed priority" means the scheduler will not permanently change the priority of a task, although it may temporarily boost the priority of a task due to priority inheritance.
"Preemptive" means the scheduler always runs the highest priority RTOS task that is able to run, regardless of when a task becomes able to run.  For example, if an interrupt service routine (ISR) changes the highest priority task that is able to run, the scheduler will stop the currently running lower priority task and start the higher priority task - even if that occurs within a time slice.  In this case, the lower priority task is said to have been "preempted" by the higher priority task.
"Round-robin" means tasks that share a priority take turns entering the Running state.
"Time sliced" means the scheduler will switch between tasks of equal priority on each tick interrupt - the time between tick interrupts being one time slice. (The tick interrupt is the periodic interrupt used by the RTOS to measure time.)


##### Using a prioritised preemptive scheduler - avoiding task starvation
A consequence of always running the highest priority task that is able to run is that a high priority task that never enters the Blocked or Suspended state will permanently starve all lower priority tasks of any execution time.  That is one reason why, normally, it is best to create tasks that are event-driven.  For example, if a high-priority task is waiting for an event, it should not sit in a loop (poll) for the event because by polling it is always running, and so never in the Blocked or Suspended state. Instead, the task should enter the Blocked state to wait for the event.  The event can be sent to the task using one of the many FreeRTOS inter-task communication and synchronisation primitives.  Receiving the event automatically removes the higher priority task from the Blocked state. Lower priority tasks will run while the higher priority task is in the Blocked state.


##### Configuring the RTOS scheduling policy
The following FreeRTOSConfig.h settings change the default scheduling behaviour:

configUSE_PREEMPTION
If configUSE_PREEMPTION is 0 then preemption is off and a context switch will only occur if the Running state task enters the Blocked or Suspended state, the Running state task calls taskYIELD(), or an interrupt service routine (ISR) manually requests a context switch.

configUSE_TIME_SLICING
If configUSE_TIME_SLICING is 0 then time slicing is off, so the scheduler will not switch between equal priority tasks on each tick interrupt.


#### The FreeRTOS AMP scheduling policy
Asymmetric multiprocessing (AMP) with FreeRTOS is where each core of a multicore device runs its own independent instance of FreeRTOS.  The cores do not all need to have the same architecture, but do need to share some memory if the FreeRTOS instances need to communicate with each other.

Each core runs its own instance of FreeRTOS so the scheduling algorithm on any given core is exactly as described above for a single-core system.  You can use a stream or message buffer as the inter-core communication primitive so that tasks on one core may enter the Blocked state to wait for data or events sent from a different core.


#### The FreeRTOS SMP scheduling policy
Symmetric multiprocessing (SMP) with FreeRTOS is where one instance of FreeRTOS schedules RTOS tasks across multiple processor cores.  As there is only one instance of FreeRTOS running, only one port of FreeRTOS can be used at a time, so each core must have the same processor architecture and share the same memory space.

The FreeRTOS SMP scheduling policy uses the same algorithm as the single-core scheduling policy but, unlike the single-core and AMP scenarios, SMP results in more than one task being in the Running state at any given time (there is one Running state task per core).  That means the assumption no longer holds that a lower priority task will only ever run when there are no higher priority tasks that are able to run.  To understand why, consider how the SMP scheduler will select tasks to run on a dual-core microcontroller when, initially, there is one high priority task and two medium priority tasks which are all in the Ready state.  The scheduler needs to select two tasks, one for each core.  First, the high priority task is the highest priority task that is able to run, so it gets selected for the first core. That leaves two medium priority tasks as the highest priority tasks that are able to run, so one gets selected for the second core. The result is that both a high and medium priority task run simultaneously.


##### Configuring the SMP RTOS scheduling policy
The following configuration options help when moving code written for single-core or AMP RTOS configurations to an SMP RTOS configuration when that code relies on the assumption that a lower priority task will not run if there is a higher priority task that is able to run.

configRUN_MULTIPLE_PRIORITIES
If configRUN_MULTIPLE_PRIORITIES is set to 0 in FreeRTOSConfig.h then the scheduler will only run multiple tasks at the same time if the tasks have the same priority.  This may fix code written with the assumption that only one task will run at a time, but only at the cost of losing some of the benefits of the SMP configuration.

configUSE_CORE_AFFINITY
If configUSE_CORE_AFFINITY is set to 1 in FreeRTOSConfig.h, then the vTaskCoreAffinitySet() API function can be used to define which cores a task can and cannot run on.  Using this, the application writer can prevent two tasks that make assumptions about their respective execution order from executing at the same time.

### Memory Management

* heap_1 - the very simplest, does not permit memory to be freed.
* heap_2 - permits memory to be freed, but does not coalescence adjacent free blocks.
* heap_3 - simply wraps the standard malloc() and free() for thread safety.
* heap_4 - coalescences adjacent free blocks to avoid fragmentation. Includes absolute address placement option.
* heap_5 - as per heap_4, with the ability to span the heap across multiple non-adjacent memory areas.
