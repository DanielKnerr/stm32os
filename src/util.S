.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb
.global SVCall

SVCall:
    svc #0
    bx lr
