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

	struct uart_msg_queue_item incoming_message;

	while (1) {
		#if 0
		// This function will not return until a new message is ready
		k_msgq_get(&uart_rx_msgq, &incoming_message, K_FOREVER);

		// Process the message here.
		static uint8_t string_buffer[UART_BUF_SIZE + 1];
		memcpy(string_buffer, incoming_message.bytes, incoming_message.length);
		string_buffer[incoming_message.length] = 0;
		printk("RX %i: %s\n", incoming_message.length, string_buffer);
		#endif
	}
}
