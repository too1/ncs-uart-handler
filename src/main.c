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

void main(void)
{
	printk("UART Async example started\n");
	
	app_uart_init();

	int counter = 0;

	while (1) {
		uint8_t *uart_rx_data;
		uint32_t uart_rx_length;
		if(app_uart_rx(&uart_rx_data, &uart_rx_length, K_MSEC(100)) == 0) {
			// Process the message here.
			printk("RX %.8x ", (uint32_t)uart_rx_data);
			for(int i = 0; i < uart_rx_length; i++) printk("%.i ", uart_rx_data[i]);
			printk("\n");
			app_uart_rx_free();
			//k_msleep(100);
		} 
		if(start_sleep_period) {
			start_sleep_period = false;
			printk("Sleep start\n");
			k_msleep(8000);
			printk("Sleep end\n");
		}
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
