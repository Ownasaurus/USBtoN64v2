.syntax unified
.globl my_wait_us_asm
.type  my_wait_us_asm, %function

my_wait_us_asm:
WAIT_1_US:
    ADD     R1, R2, #0      // clock cycle 1
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 10
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 20
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 30
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 40
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 50
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 60
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0
    ADD     R1, R2, #0      // clock cycle 70

    SUBS    R0, R0, #1      // clock cycle 71
    BGT     WAIT_1_US       // clock cycle 72

    BX      LR      // return
