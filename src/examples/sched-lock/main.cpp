/*___________________________________________________________________________*/

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "avrtos/misc/uart.h"
#include "avrtos/misc/led.h"

#include "avrtos/kernel.h"
#include "avrtos/debug.h"

/*___________________________________________________________________________*/

void thread_blink(void *p);
void thread_coop(void *p);

K_THREAD_DEFINE(blink, thread_blink, 0x100, K_PRIO_PREEMPT(K_PRIO_HIGH), nullptr, nullptr, 'B');
K_THREAD_DEFINE(coop, thread_coop, 0x100, K_PRIO_COOP(K_PRIO_HIGH), nullptr, nullptr, 'C');

/*___________________________________________________________________________*/

int main(void)
{
  led_init();
  usart_init();
  
  k_thread_dump_all();

  print_runqueue();

  sei();

  while(1)
  {
    k_sched_lock();
    usart_printl("k_sched_lock()");

    _delay_ms(500);

    usart_printl("k_sched_unlock()");
    k_sched_unlock();

    _delay_ms(2000);
  }
}

void thread_blink(void *p)
{
  while(1)
  {
    usart_transmit('o');
    led_on();

    k_sleep(K_MSEC(100));

    usart_transmit('f');
    led_off();

    k_sleep(K_MSEC(100));
  }
}

void thread_coop(void *p)
{
  while(1)
  {
    k_sleep(K_MSEC(10000));

    usart_printl("full cooperative thread");

    _delay_ms(500);
  }
}

/*___________________________________________________________________________*/