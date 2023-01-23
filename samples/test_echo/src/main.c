/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <uart_handler.h>

#define SLEEP_TIME_MS 1000

static void on_app_uart_event(struct app_uart_evt_t *evt)
{
	switch(evt->type) {
		// Data received over the UART interface. 
		// NOTE: The UART data buffers are only guaranteed to be retained until this function ends
		//       If the data can not be processed immediately, they should be copied to a different buffer
		case APP_UART_EVT_RX:
			// Simply echo any incoming data back to the sender
			app_uart_send(evt->data.rx.bytes, evt->data.rx.length, K_FOREVER); 
			break;

		// A UART error ocurred, such as a break or frame error condition
		case APP_UART_EVT_ERROR:
			printk("UART error: Reason %i\n", evt->data.error.reason);
			break;

		// The UART event queue has overflowed. If this happens consider increasing the UART_EVENT_QUEUE_SIZE (will increase RAM usage),
		// or increase the UART_RX_TIMEOUT_US parameter to avoid a lot of small RX packets filling up the event queue
		case APP_UART_EVT_QUEUE_OVERFLOW:
			printk("UART library error: Event queue overflow!\n");
			break;
	}
}

void main(void)
{
	int err;

	printk("App Uart echo test sample started\n");
	
	err = app_uart_init(on_app_uart_event);
	if(err != 0) {
		printk("app_uart_init failed: %i\n", err);
		return;
	}

	while(1) {
		k_msleep(SLEEP_TIME_MS);
	}
}
