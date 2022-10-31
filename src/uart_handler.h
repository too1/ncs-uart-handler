#ifndef __UART_HANDLER_H
#define __UART_HANDLER_H

#include <zephyr.h>
#include <device.h>

#define UART_BUF_SIZE		16
#define UART_TX_TIMEOUT_MS	100
#define UART_RX_TIMEOUT_MS	100

#define UART_TX_BUF_SIZE  		256
#define UART_RX_MSG_QUEUE_SIZE	8

struct uart_msg_queue_item {
	uint8_t bytes[UART_BUF_SIZE];
	uint32_t length;
};

void app_uart_init(void);

int app_uart_send(const uint8_t * data_ptr, uint32_t data_len);

#endif