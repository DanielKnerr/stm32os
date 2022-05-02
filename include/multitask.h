#pragma once
#include <stdint.h>
#include <stdbool.h>

enum TaskStates { READY, RUNNING, SLEEPING, DEAD };

struct TCB {
    uint32_t *stack, *kernelStack;
    uint32_t *stackPointer, *kernelStackPointer;
    uint32_t stackSize, kernelStackSize;
    enum TaskStates state;
    uint32_t sleepUntil;
    int priority;
    uint32_t usedSlices;
    uint32_t stoppedAt;
    bool waitingForPPM;
};

extern uint32_t *stackPointerA, *stackPointerB;
extern uint32_t *kernelStackPointerA, *kernelStackPointerB;

void initTasks();
void addTask(void *taskEntry, int priority);
void idleTask();
void delayCurrentTaskUntil(uint32_t ms);
void delayTaskUntilPPM();
void killCurrentTask();

uint32_t *onSysTick(uint32_t *i, uint32_t currentMillis);
