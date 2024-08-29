/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <avrtos/avrtos.h>
#include <avrtos/debug.h>
#include <avrtos/drivers/usart.h>

#include <avr/interrupt.h>
#include <avr/io.h>

const char message[] = "Hello World!\n";

int main(void)
{
	/* UART initialisation */
	const struct usart_config usart_config = {
		.baudrate	 = USART_BAUD_115200,
		.receiver	 = 1u,
		.transmitter = 1u,
		.mode		 = USART_MODE_ASYNCHRONOUS,
		.parity		 = USART_PARITY_NONE,
		.stopbits	 = USART_STOP_BITS_1,
		.databits	 = USART_DATA_BITS_8,
		.speed_mode	 = USART_SPEED_MODE_NORMAL,
	};
	ll_usart_init(USART0_DEVICE, &usart_config);

	k_thread_dump_all();

	for (;;) {
		usart_send(USART0_DEVICE, message, strlen(message));

		k_sleep(K_SECONDS(1));
	}
}