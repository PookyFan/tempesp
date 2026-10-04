#include "FreeRTOS.h"
#include "utils.h"

static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
static StaticTask_t idle_task_tcb;

void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &idle_task_tcb;
    *ppxIdleTaskStackBuffer = idle_task_stack;
    *pulIdleTaskStackSize = ARRAY_SIZE(idle_task_stack);
}

static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];
static StaticTask_t timer_task_tcb;

void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &timer_task_tcb;
    *ppxIdleTaskStackBuffer = timer_task_stack;
    *pulIdleTaskStackSize = ARRAY_SIZE(timer_task_stack);
}