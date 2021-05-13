#include <avr/io.h>

.global k_thread_switch
.global k_thread_stack_create
.global k_yield
.global k_schedule

.extern k_thread

#if THREAD_USE_INIT_STACK_ASM

; thread_t *th         in r24, r25
; thread_entry_t entry in r22, r23
; uint16_t stack_end   in r20, r21
; void* context_p      in r18, r19
k_thread_stack_create:
    push r26
    push r27
    push r28
    push r29

    ; load stack pointer in X register
    ; mov r22, r23 to X
    movw r26, r20

    ; add 1 to stack pointer in order
    adiw r26, 1

    ; addr of function is already shifted
    ; "return addr" 1 bit shift (>>1)
    ; lsr r23
    ; ror r22

    ; add return addr to stack (with format >> 1)
    st -X, r22 ; SPL
    st -X, r23 ; SPH

    ; push 30 default registers (r1 == 0 for example, ...) + pass (void * context)
    ; r0 > r23 (24 registers)
    ldi r28, 0x00
    ldi r29, 24
    dec r29
    st -X, r28
    brne .-6

    ; void * context ~ push r24 > r25
    st -X, r18
    st -X, r19

    ; push r26 > r31 (6 registers)
    ldi r29, 6
    dec r29
    st -X, r28
    brne .-6
    
    ; push sreg default (0)
    lds r29, SREG
    st -X, r29

    ; copy stack pointer in thread structure

    ; -1, to first empty stack addr
    sbiw r26, 1

    ; mov r24, r25 (thread_t *th) to Y
    movw r28, r24

    ; save SP in thread structure
    st Y+, r26
    st Y, r27

    pop r29
    pop r28
    pop r27
    pop r26

    ret  ; dispath to next thread

#endif

/*___________________________________________________________________________*/

; thread_t *from   in r24, r25
; thread_t *to     in r22, r23
k_thread_switch:
    ; save current thread registers
    ; push 32 registers
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21

    ; as the function has 2 (void* = 16bits) arguments
    ; these 4 registers are already save onto the stack

    ; ?
    push r22
    push r23
    push r24
    push r25

    push r26
    push r27
    push r28
    push r29
    push r30
    push r31

    ; push SREG onto stack;
    lds r0, SREG
    push r0

    ; disable interrupts
    ; TODO can be done later
    
    ; save stack pointer in structure (thread_t *from)
    cli
    lds r0, SPL
    sei
    lds r1, SPH

    ; set structure pointer to X
    movw r26, r24

    st X+, r0    ; write SPL in uint16_t low byte (?)
    st X+, r1    ; write SPH in uint16_t high byte (?)

    ; restore the other thread

    ; restore stack pointer from  structure (thread_t *from)

    ; set structure pointer to X
    movw r26, r22

    ld r0, X+  ; low byte
    ld r1, X+  ; high byte

    ; todo cli here

    ; set stack pointer
    cli
    sts SPL, r0
    sei ; set interrupt mask
    sts SPH, r1

    ; restore SREG from stack;
    pop r0
    sts SREG, r0

    ; restore 32 registers
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26

    ; as the function has 2 (void* = 16bits) arguments
    ; these 4 registers are already saved
    ; no need to restore them

    ; ?
    pop r25
    pop r24
    pop r23
    pop r22

    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    ret ; 2 following Bytes in stack are return address

/*___________________________________________________________________________*/

; see k_thread_switch (th* a -> th* b)
k_yield:
    ; save current thread registers
    ; push 32 registers
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r24
    push r25
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31

    lds r0, SREG    ; save flags
    push r0

    cli
    lds r0, SPL
    sei
    lds r1, SPH

    lds r26, k_thread           ; load current thread structure addr in X
    lds r27, k_thread + 1

    st X+, r0   ; save SP
    st X+, r1

    ; determine which is the next thread
    call k_schedule

    ; next thread structure addr is in X
    movw r26, r24

    ld r0, X+
    ld r1, X+

    cli
    sts SPL, r0 ; restore stack pointer
    sei
    sts SPH, r1

    pop r0  ; restore flags
    sts SREG, r0

    ; restore 32 registers
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    pop r25
    pop r24
    pop r23
    pop r22
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    ret  ; dispath to next thread