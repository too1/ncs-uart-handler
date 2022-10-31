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

void main(void)
{
	printk("UART Async example started\n");
	
	app_uart_init();

	const uint8_t test_string[] = "Hello world through the UART async driver\r\n";
	app_uart_send(test_string, strlen(test_string));

	while (1) {
		uint8_t *uart_rx_data;
		uint32_t uart_rx_length;
		if(app_uart_rx(&uart_rx_data, &uart_rx_length, K_FOREVER) == 0) {
			// Process the message here.
			static uint8_t string_buffer[64];
			memcpy(string_buffer, uart_rx_data, uart_rx_length);
			string_buffer[uart_rx_length] = 0;
			printk("RX %i: %s\n", uart_rx_length, string_buffer);
		} 
		k_msleep(2000);
	}
}
