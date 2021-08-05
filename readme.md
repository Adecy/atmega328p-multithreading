# AVRTOS : RTOS attempt for 8bits AVR microcontrollers

## Getting started example :

### Description

In this example, we spawn two threads (+ main thread) :
- One thread set the led on for 500ms and ther other off for the same time
- A mutex is protecting the LED access, and both threads keeps the mutex locked while waiting
- The main thread release the CPU forever when initialization finished

On an Arduino Pro (ATmega328p, avr5) the led should be blinking at the frequency of 1Hz.

Configuration option : `CONFIG_KERNEL_TIME_SLICE=10`

### Code

```cpp
#include <avr/io.h>
#include "avrtos/misc/uart.h"
#include "avrtos/misc/led.h"
#include "avrtos/kernel.h"
#include "avrtos/debug.h"

void thread_led(void *context);
void thread_coop(void *context);

uint8_t on = 1u;
uint8_t off = 0u;

K_MUTEX_DEFINE(mymutex);  // mutex protecting LED access
K_THREAD_DEFINE(ledon, thread_led, 0x50, K_PRIO_PREEMPT(K_PRIO_HIGH), (void *)&on, nullptr, 'O');
K_THREAD_DEFINE(ledoff, thread_led, 0x50, K_PRIO_PREEMPT(K_PRIO_HIGH), (void *)&off, nullptr, 'F');

int main(void)
{
  led_init();
  usart_init();
  k_thread_dump_all();
  sei();
  k_sleep(K_FOREVER);
}

void thread_led(void *context)
{
  const uint8_t thread_led_state = *(uint8_t*)context;
  while(1)
  {
    k_mutex_lock_wait(&mymutex, K_FOREVER);
    led_set(thread_led_state);
    usart_transmit(thread_led_state ? 'o' : 'f');
    k_sleep(K_MSEC(500));
    k_mutex_release(&mymutex);
  }
}
```

### Logs

```
===== k_thread =====
M 021C [PREE 0] RUNNING : SP 0/512 -| END @045E
F 022C [PREE 1] READY   : SP 35/80 -| END @0157
O 023C [PREE 1] READY   : SP 35/80 -| END @01A7
K 024C [PREE 3] READY   : SP 35/62 -| END @01E6
fofofofofofofofofofofofofofofofofofofofofofofofofofofo
```

## Introduction

This project is an attempt to create a real time operating system for AVR 8 bits microcontrollers.

Following avr architectures are supported/tested :
- avr5, especially ATmega328p : tested with an Arduino PRO
- avr6, especially ATmega2560 : tested with an Arduino Mega2560

Following features are supported:
- Cooperative threads
- Preemptive threads with time slice between 1ms and 16ms
- Mutex
- Thread sleeping up to 65 seconds (or more if using high precision time objects)
- Scheduler lock/unlock to temporarily set a preemptive thread as cooperative
- Waking up threads waiting on events (mutex release)
- Scheduler calls (thread switching) from interrupt handler
- Canaries
- Runtime stack/thread creation
- avr5 and avr6 architectures

Minor features:
- thread naming with a symbol, e.g. 'M' for the main thread 'I' for the idle thread 
- thread local storage
- thread context passing
- debug/utils functions : RAM_DUMP, CORE_DUMP, read_ra (read return address)
- data structures : dlist (doubly linked list), queue (singly linked list), time queue (singly linked list with delay parameter)
- I/O : leds, uart

What paradigms are not supported:
- Nested interrupts

What features will be implemented :
- Prioritization
- Semaphores
- Signals
- System workqueue and work items
- Delayed start, suspending/resuming threads
- Stack sentinels
- Pseudo random number generator : [LFSR](https://es.wikipedia.org/wiki/LFSR)

What enhancements are planned :
- removing CPU idle thread
- don't keep the idle thread in the runqueue if others threads are in
- using makefile to build the project for a target

Inspiration in the naming comes greatly from the project [Zephyr RTOS](https://github.com/zephyrproject-rtos/zephyr) projects 
as well as some paradigms and concepts regarding multithreading : [Zephyr : Threads](https://docs.zephyrproject.org/latest/reference/kernel/threads/index.html).

## PlatformIO

- This project was developp using PlatformIO environnement in Visual Studio Code IDE.
- Each example can be easily built using the *pio* configuration file :
    - Board, e.g. : pro16MHzatmega328, megaatmega2560
    - Choose the linker script depending on the AVR architecture
    - Choose *upload* and *monitor* ports

```
[env]
platform = atmelavr
board = pro16MHzatmega328
; board = megaatmega2560
framework = arduino

src_filter =
    +<avrtos/>

build_flags = 
    -Wl,-T./avrtos-avr5.xn
    ; -Wl,-T./avrtos-avr6.xn

upload_port = COM3

monitor_port = COM3
monitor_speed = 500000
```

## Configuration options

| Configuration option                   | Description                                                                                                                                                   | default                         | min | max   |
| -------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------- | --- | ----- |
| THREAD\_MAX                            | Define the maximum number of threads supported                                                                                                                | 5                               | 2   | ?     |
| THREAD\_MAIN\_THREAD\_PRIORITY         | Define the main thread type (coop/prempt) and priority                                                                                                        | K\_PRIO\_PREEMPT(K\_PRIO\_HIGH) | 0   | 0b111 |
| THREAD\_EXPLICIT\_MAIN\_STACK          | Tells if the main stack location and size must be defined at compilation time (1) or if the default main stack behaviour (stack at RAMEND) should be kept (0) | 1                               | 0   | 1     |
| THREAD\_MAIN\_STACK\_SIZE              | In the case we defined (EXPLICIT\_MAIN\_STACK == 1), this configuration option defines the size of the main stack                                             | 0x200                           | 0   | ?     |
| THREAD\_USE\_INIT\_STACK\_ASM          | Tells if we should use the C or the Assembler function to define our threads at runtime                                                                       | 1                               | 0   | 1     |
| THREAD\_DEFAULT\_SREG                  | Default SREG value for other thread on stack creation. Main thread default SREG is always 0.                                                                  | 1<< SREG\_I                     | 0   | 0xFF  |
| THREAD\_CANARIES                       | Enable thread canaries                                                                                                                                        | 0                               | 0   | 1     |
| THREAD\_CANARIES\_SYMBOL               | Define thread canaries symbol                                                                                                                                 | 0xAA                            | 0   | 0xFF  |
| THREAD\_SYMBOL                         | Tell if we can name threads using a 2 letters symbol (e.g. M, T1, T2, ...)                                                                                    | 1                               | 1   | 1     |
|                                        |                                                                                                                                                               |                                 |     |       |
| KERNEL\_HIGH\_RANGE\_TIME\_OBJECT\_U32 | Configure to use uint32\_t as k\_delta\_ms\_t ~= 50 days or keep (uint16\_t) ~= 65seconds                                                                     | 0                               | 0   | 1     |
| KERNEL\_PREEMPTIVE\_THREADS            | Enable preemtive threads feature                                                                                                                              | 1                               | 0   | 1     |
| KERNEL\_TIME\_SLICE                    | Time slice in milliseconds                                                                                                                                    | 4                               | 1   | 16    |
| KERNEL\_SYSCLOCK\_AUTO\_INIT           | Auto start kernel sysclock                                                                                                                                    | 1                               | 0   | 1     |
| KERNEL\_THREAD\_IDLE                   | Tells if the kernel should define a idle thread to permit all user defined threads to be in waiting status                                                    | 1                               | 0   | 1     |
| KERNEL\_THREAD\_IDLE\_ADD\_STACK       | Kernel thread idle stack size                                                                                                                                 | 0                               | 0   | ?     |
| KERNEL\_ALLOW\_INTERRUPT\_YIELD        | Allow interrupt yield, this forces to add more stack to idle thread                                                                                           | 1                               | 0   | 1     |
|                                        |                                                                                                                                                               |                                 |     |       |
| KERNEL\_DEBUG\_PREEMPT\_UART           | Use uart rx interrupt as preempt signal                                                                                                                       | 0                               | 0   | 1     |
| KERNEL\_DEBUG                          | Enable Kernel Debug features                                                                                                                                  | 0                               | 0   | 1     |
| KERNEL\_SCHEDULER\_DEBUG               | Enable Kernel Debug in scheduler                                                                                                                              | 0                               | 0   | 1     |

## Debugging

Enabling configuration option `KERNEL_SCHEDULER_DEBUG` enables following logs :
- Before scheduler call :
    - `c` : Meaning that an interrupt planned to call the scheduler but the current thread is in cooperative mode. We restore the current thread same context that was only partially saved.
    - `s` : Afther the scheduler get called, there is still only one thread that is ready to be executed, and we restore the context that was partially saved.
- During scheduler call :
    - `!` : The timer of a waiting thread expired and the scheduler poped it off in order to execute it.
    - `~` : Current thread has just been set in WAITING mode and has been removed from the runqueue. The scheduler get the next thread to be executed.
    - `>` : The scheduler simply requeue the current thread and get the next thread to be executed.
    - `p` : Next thread to be executed is the IDLE thread, since there are other threads to be executed we skip the IDLE thread.
    - `M`, `K`, `A`, *Any* : After one of the symbols described above, the next thread symbol is printed in the console.
- On mutex handling:
    - `#` : mutex is available and the current thread get it without waiting
    - `{X#`: mutex is already locked and the current thread `X` needed to wait `{` before getting it `#`
    - `{X*`: the current thread is locking the mutex, the thread `X` is waiting on it, we release it and we wake up this thread `*`

Enabling configuration option `KERNEL_SCHEDULER_DEBUG` enables following logs :
- `.` : Each time the syslock timer expires, we check if the current thread can be preempted and then the scheduler is called in.
    - If configuration option `KERNEL_DEBUG_PREEMPT_UART` is set, syslock timer overflow handler is replaced by the uart rx interrupt handler.

### Example with the getting-started example 

```
===== k_thread =====
M 021E [PREE 0] RUNNING : SP 0/512 -| END @0460
F 022E [PREE 1] READY   : SP 35/80 -| END @0157
O 023E [PREE 1] READY   : SP 35/80 -| END @01A7
K 024E [PREE 3] READY   : SP 35/62 -| END @01E6
~F#f~O~K.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.!F{F*>O{O#o~pF~K.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.!O{O*>F{F#f~pO~K.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.!F{F*>O{O#o~pF~K.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.!O{O*>F{F#f~pO~K.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks.>Ks
```

## Things ...

- didn't use any debugger to develop this project
- AVRTOS stands for AVR and RTOS

## Some links :
- More information on data structures : https://en.wikipedia.org/wiki/Linked_list
- Memory sections on AVR : https://www.nongnu.org/avr-libc/user-manual/mem_sections.html
- avr-gcc : https://gcc.gnu.org/wiki/avr-gcc
- Pseudo-ops and operators : https://www.nongnu.org/avr-libc/user-manual/assembler.html#ass_pseudoop
- AVR Instruction Set Manual : http://ww1.microchip.com/downloads/en/devicedoc/atmel-0856-avr-instruction-set-manual.pdf