#ifndef __UART_HANDLER_H
#define __UART_HANDLER_H

#include <zephyr.h>
#include <device.h>
#include <drivers/uart.h>

#define UART_RX_TIMEOUT_US	100000

#define UART_TX_BUF_SIZE  				256
#define UART_RX_BUF_SIZE				256

#define UART_RX_MSG_QUEUE_SIZE			32
#define UART_EVENT_THREAD_STACK_SIZE 	1024
#define UART_EVENT_THREAD_PRIORITY		5

enum app_uart_evt_types_t {APP_UART_EVT_RX, APP_UART_EVT_ERROR, APP_UART_EVT_QUEUE_OVERFLOW};

struct uart_msg_queue_item {
	enum app_uart_evt_types_t type;
	union {
		struct {
			uint8_t *bytes;
			uint32_t length;
			uint8_t *_source_buf;
		} rx;
		struct {
			enum uart_rx_stop_reason reason;
		} error;
	} data;
};

typedef void (*app_uart_event_handler_t)(struct uart_msg_queue_item *evt);

int app_uart_init(app_uart_event_handler_t evt_handler);

int app_uart_send(const uint8_t * data_ptr, uint32_t data_len, k_timeout_t timeout);

int app_uart_rx(uint8_t ** data_ptr, uint32_t * data_len, k_timeout_t timeout);

int app_uart_rx_free(void);

#endif