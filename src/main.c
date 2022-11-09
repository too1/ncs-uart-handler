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

static void on_app_uart_event(struct uart_msg_queue_item *evt)
{
	switch(evt->type) {
		case APP_UART_EVT_RX:
			printk("RX ");
			for(int i = 0; i < evt->data.rx.length; i++) printk("%.i ", evt->data.rx.bytes[i]);
			printk("\n");
			break;

		case APP_UART_EVT_ERROR:
			printk("UART error: Reason %i\n", evt->data.error.reason);
			break;

		case APP_UART_EVT_QUEUE_OVERFLOW:
			printk("UART error: Event queue overflow!\n");
			break;
	}
}

void main(void)
{
	printk("UART Async example started\n");
	
	app_uart_init(on_app_uart_event);

	while (1) {
		/*if(start_sleep_period) {
			start_sleep_period = false;
			printk("Sleep start\n");
			k_msleep(8000);
			printk("Sleep end\n");
		}*/
		k_msleep(1000);
	}
}

static void thread_test_uart_tx(void)
{
	int sleep_time = 250;
	int runtime = 0;
	static uint8_t cnt = 0;
	while(1) {
		for(int i = 0; i < TEST_LENGTH; i++) test_buf[i] = cnt++;
		app_uart_send(test_buf, TEST_LENGTH, K_NO_WAIT);
		k_msleep(sleep_time);
		runtime += sleep_time;
		if(runtime > 20000) {
			start_sleep_period = true;
			runtime = 0;
		}
	}
} 

K_THREAD_DEFINE(thread_tx, 512, thread_test_uart_tx, 0, 0, 0, 5, 0, 0);  
