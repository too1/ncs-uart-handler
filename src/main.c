/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <sys/ring_buffer.h>
#include <drivers/gpio.h>
#include <uart_handler.h>
#include <string.h>

#define TEST_LENGTH 10
static uint8_t test_buf[TEST_LENGTH];

static volatile bool start_sleep_period = false;

static void on_app_uart_event(struct app_uart_evt_t *evt)
{
	switch(evt->type) {
		// Data received over the UART interface. 
		// NOTE: The UART data buffers are only guaranteed to be retained until this function ends
		//       If the data can not be processed immediately, they should be copied to a different buffer
		case APP_UART_EVT_RX:
			printk("RX ");
			for(int i = 0; i < evt->data.rx.length; i++) printk("%.i ", evt->data.rx.bytes[i]);
			printk("\n");
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

	printk("UART Async example started\n");
	
	err = app_uart_init(on_app_uart_event);
	if(err != 0) {
		printk("app_uart_init failed: %i\n", err);
		return;
	}

	int sleep_time = 250;
	static uint8_t cnt = 0;
	while(1) {
		for(int i = 0; i < TEST_LENGTH; i++) test_buf[i] = cnt++;
		app_uart_send(test_buf, TEST_LENGTH, K_NO_WAIT);
		k_msleep(sleep_time);
	}
}
