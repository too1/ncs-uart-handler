/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <uart_handler.h>

#define SLEEP_TIME_MS 4000

static uint8_t test_buf_tx[] = "Hello world from the app_uart driver!!\r\n";

static void on_app_uart_event(struct app_uart_evt_t *evt)
{
	switch(evt->type) {
		// Data received over the UART interface. 
		// NOTE: The UART data buffers are only guaranteed to be retained until this function ends
		//       If the data can not be processed immediately, they should be copied to a different buffer
		case APP_UART_EVT_RX:
			printk("RX (%i bytes): %.*s\n", evt->data.rx.length, evt->data.rx.length, evt->data.rx.bytes); 
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

int main(void)
{
	int err;

	printk("UART Async example started\n");
	
	err = app_uart_init(on_app_uart_event);
	if(err != 0) {
		printk("app_uart_init failed: %i\n", err);
		return 0;
	}

	while(1) {
		app_uart_send(test_buf_tx, strlen(test_buf_tx), K_NO_WAIT);
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
