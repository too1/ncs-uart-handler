#ifndef __UART_HANDLER_H
#define __UART_HANDLER_H

#include <zephyr.h>
#include <device.h>

#define UART_RX_TIMEOUT_US	100000

#define UART_TX_BUF_SIZE  		256
#define UART_RX_BUF_SIZE		256
#define UART_RX_MSG_QUEUE_SIZE	16

struct uart_msg_queue_item {
	uint8_t *bytes;
	uint32_t length;
};

void app_uart_init(void);

int app_uart_send(const uint8_t * data_ptr, uint32_t data_len);

int app_uart_rx(uint8_t ** data_ptr, uint32_t * data_len, k_timeout_t timeout);

#endif