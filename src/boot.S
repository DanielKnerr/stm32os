.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb
.global _entry
.global main
.global interruptHandler

.section    .text
.weak _reset
.type     _reset, %function
_reset:
    @ blx zum Springen nutzen, da der Linker "_entry" und "main" evtl. zu weit weg ablegt
    @ erst den Bootstrapper ausführen
    ldr r0, =_entry
    blx _entry
    @ dann in den Kernel springen
    ldr r0, =main
    blx main

    @ den Kernel neustarten falls er crasht
    b _reset

.type     handler, %function
handler:
    push {r4-r11}
    push {lr}
    mrs r0, ipsr
    push {r0}
    mrs r0, psp
    push {r0}
    mov r0, sp
    bl interruptHandler
    mov sp, r0
    pop {r0}
    msr psp, r0
    pop {r0}
    pop {lr}
    pop {r4-r11}
    bx lr

.global setPSP
setPSP:
    msr psp, r0
    bx lr

@ interrupt vector table
@ durch "a" wird dieser section in den Speicher geladen 
@ "%progbits" um zu deklarieren, dass diese section Daten enthält

.section  .int_vector_table, "a", %progbits

basic_vectors:
    .word   _estack             /* die "top of stack" Adresse - also die letzte SRAM Adresse - (in basic.ld deklariert) */
    .word   _reset              /* die Adresse des Reset Handler */

    @ alle Interrupts sollen den selben Interrupt Handler nutzen:
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
    .word handler
