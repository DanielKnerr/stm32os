#include "../include/multitask.h"
#include "../include/register.h"
#include "../include/interrupts.h"
#include "../include/memory.h"
#include "../include/gpio.h"
#include "../include/drivers/i2c.h"
#include "../include/drivers/bno055.h"
#include "../include/drivers/nrf24l01.h"
#include "../include/drivers/ppm.h"
#include "../include/syscall.h"
#include <stdbool.h>

#define INITIAL_TCB_SIZE 5

struct TCB *tcbArray;
uint16_t tcbSize, tcbContents;

uint32_t *stackA, *stackB;
uint32_t *kernelStackA, *kernelStackB;
uint32_t *stackPointerA, *stackPointerB;
uint32_t *kernelStackPointerA, *kernelStackPointerB;
struct inter interA, interB;

void initTasks() {
    tcbArray = (struct TCB*) allocate(sizeof(struct TCB) * INITIAL_TCB_SIZE);
    tcbSize = INITIAL_TCB_SIZE;
    tcbContents = 0;

    addTask(idleTask, 99);
}

void addTask(void *taskEntry, int priority) {
    if (tcbContents == tcbSize) {
        tcbSize++;
        // TODO: reallocate implementieren
        // tcbArray = reallocate(sizeof(struct TCB) * tcbSize);
    }

    #define STACK_SIZE 512

    struct TCB *tcb = &tcbArray[tcbContents];
    tcb->stack = (uint32_t*) allocate(STACK_SIZE);
    tcb->stackPointer = tcb->stack + STACK_SIZE;
    tcb->stackSize = STACK_SIZE;
    tcb->kernelStack = (uint32_t*) allocate(STACK_SIZE);
    tcb->kernelStackPointer = tcb->kernelStack + STACK_SIZE;
    tcb->kernelStackSize = STACK_SIZE;
    tcb->state = READY;
    tcb->priority = priority;
    tcb->sleepUntil = 0;
    tcb->stoppedAt = 0;
    tcb->usedSlices = 0;
    tcb->waitingForPPM = false;

    // die Größe der structs durch die Größe eines uint32_t teilen,
    // da stackPointer und kernelStackPointer Arrays von Typ uint32_t sind
    tcb->stackPointer -= sizeof(struct interByCPU) / sizeof(uint32_t);
    tcb->kernelStackPointer -= sizeof(struct interCustom) / sizeof(uint32_t);
    struct interByCPU *PSP = (struct interByCPU *)tcb->stackPointer;
    struct interCustom *MSP = (struct interCustom *) tcb->kernelStackPointer;
    PSP->PC = (uint32_t) taskEntry;
    PSP->xPSR = 0x1000000;
    MSP->EXC_RETURN = 0xFFFFFFFD;
    MSP->PSP = (uint32_t) tcb->stackPointer;

    // damit der Task beendet wird, wenn die Task Funktion ein return macht, muss
    // die Addresse einer Funktion im Link Register sein, die den Task beendet 
    PSP->LR = exitTask;

    tcbContents++;
}

void idleTask() {
    while (1) {}
}

int currentTaskIdx = -1;
int prevTaskIdx = -1;

// Alle Tasks durchlaufen und die, die nicht mehr warten müssen auf READY setzen 
void updateTasks(uint32_t currentMillis) {
    for (int i = 0; i < tcbContents; i++) {
        if (tcbArray[i].waitingForPPM) {
            if (ppmAvailable) {
                if (tcbArray[i].state == SLEEPING && tcbArray[i].waitingForPPM == true) {
                    tcbArray[i].state = READY;
                    tcbArray[i].waitingForPPM = false;
                }
            }
        } else {
            if (tcbArray[i].state == SLEEPING && tcbArray[i].sleepUntil <= currentMillis) {
                tcbArray[i].state = READY;
            }
        }

        // TODO: Es kann sein, dass nach dem Aufwecken eines Tasks, der auf ein PPM Paket wartet,
        // ein neues PPM Paket (von dem noch gar nicht bekannt ist, ob es gültig ist) in das ppmValues Array geschrieben wird.
    }
}

uint32_t maxSlices = 2;
uint32_t selectTask(uint32_t currentMillis) {
    // wenn der aktuelle Task noch READY ist und seine time-slice nicht hat kann er einfach weiterarbeiten
    // TODO: der idle task soll niemals "einfach so" seine time slice ausnutzen dürfen!
    if (currentTaskIdx != -1) {
        if (tcbArray[currentTaskIdx].state == READY && tcbArray[currentTaskIdx].usedSlices <= maxSlices) {
            return currentTaskIdx;
        } else {
            tcbArray[currentTaskIdx].stoppedAt = currentMillis;
        }
    }

    // wählt den Task aus, der schon für die längste Zeit nicht mehr aktiv war
    // das klappt nicht, wenn oldestTaskMs und oldestTaskIdx auf 0 initialisiert werden
    int oldestTaskMs = -1, oldestTaskIdx = -1;
    for (int i = 0; i < tcbContents; i++) {
        if (tcbArray[i].state == READY && tcbArray[i].stoppedAt < oldestTaskMs) {
            oldestTaskMs = tcbArray[i].stoppedAt;
            oldestTaskIdx = i;
        }
    }

    return oldestTaskIdx;
}

uint32_t *onSysTick(uint32_t *ptr, uint32_t currentMillis) {
    updateTasks(currentMillis);

    int nextTask = selectTask(currentMillis);

    if (nextTask == currentTaskIdx) {
        tcbArray[currentTaskIdx].usedSlices++;
        return ptr;
    } else {
        if (currentTaskIdx != -1) {
            prevTaskIdx = currentTaskIdx;
        }

        if (prevTaskIdx != -1) {
            (&tcbArray[prevTaskIdx])->kernelStackPointer = ptr;
            (&tcbArray[prevTaskIdx])->stackPointer = ptr + 1;//interrupt.custom->PSP;
        }

        tcbArray[currentTaskIdx].stoppedAt = currentMillis;
        tcbArray[currentTaskIdx].usedSlices = 0;

        currentTaskIdx = nextTask;
        uint32_t *newMSP = tcbArray[currentTaskIdx].kernelStackPointer;
        prevTaskIdx = currentTaskIdx;
        return newMSP;
    }
}

void delayCurrentTaskUntil(uint32_t ms) {
    tcbArray[currentTaskIdx].sleepUntil = ms;
    tcbArray[currentTaskIdx].state = SLEEPING;
}

void delayTaskUntilPPM() {
    tcbArray[currentTaskIdx].state = SLEEPING;
    tcbArray[currentTaskIdx].waitingForPPM = true;

    // aktiviert Interrupts für EXTI0
    *EXTI_IMR |= 1;
}

void killCurrentTask() {
    tcbArray[currentTaskIdx].state = DEAD;
}
