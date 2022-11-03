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

#define TEST_LENGTH 1024

void main(void)
{
	printk("UART Async example started\n");
	
	app_uart_init();

	int counter = 0;
	static uint8_t test_buf[TEST_LENGTH];
	for(int i = 0; i < TEST_LENGTH; i++) test_buf[i] = (i / 4);

	while (1) {
		uint8_t *uart_rx_data;
		uint32_t uart_rx_length;
		if(app_uart_rx(&uart_rx_data, &uart_rx_length, K_MSEC(100)) == 0) {
			// Process the message here.
			static uint8_t string_buffer[65];
			memcpy(string_buffer, uart_rx_data, uart_rx_length);
			string_buffer[uart_rx_length] = 0;
			printk("RX %i: %.2x-%.2x-%.2x-%.2x\n", uart_rx_length, uart_rx_data[0], uart_rx_data[1], uart_rx_data[2], uart_rx_data[3]);
			app_uart_rx_free(uart_rx_length);
			k_msleep(100);
		} 
		if(counter++ > 100) {
			counter = 0;
			app_uart_send(test_buf, TEST_LENGTH, K_MSEC(100));
		}
	}
}
