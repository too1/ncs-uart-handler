#include "uart_handler.h"
#include <sys/ring_buffer.h>
#include <drivers/uart.h>
#include <string.h>

K_SEM_DEFINE(tx_done, 1, 1);

// UART TX fifo
RING_BUF_DECLARE(app_tx_fifo, UART_TX_BUF_SIZE);
volatile int bytes_claimed;

// UART RX primary buffers
#define UART_RX_DBL_BUF_SIZE (UART_RX_BUF_SIZE / 4)
uint8_t uart_rx_buffer[4][UART_RX_DBL_BUF_SIZE];
uint32_t uart_rx_buf_index = 0;
#define UART_RX_NEXT_BUF &uart_rx_buffer[uart_rx_buf_index][0]
#define UART_RX_BUF_INC() uart_rx_buf_index = (uart_rx_buf_index + 1) % 4

// UART RX message queue
K_MSGQ_DEFINE(uart_rx_msgq, sizeof(struct uart_msg_queue_item), UART_RX_MSG_QUEUE_SIZE, 4);

// UART RX free bytes
static uint32_t free_rx_bytes = UART_RX_BUF_SIZE;
static bool rx_enable_request = false;
static bool rx_received_since_enable = false;

static const struct device *dev_uart;

static int uart_tx_get_from_queue(void)
{
	uint8_t *data_ptr;
	// Try to claim any available bytes in the FIFO
	bytes_claimed = ring_buf_get_claim(&app_tx_fifo, &data_ptr, UART_TX_BUF_SIZE);

	if(bytes_claimed > 0) {
		// Start a UART transmission based on the number of available bytes
		uart_tx(dev_uart, data_ptr, bytes_claimed, SYS_FOREVER_MS);
	}
	return bytes_claimed;
}

void app_uart_async_callback(const struct device *uart_dev,
							 struct uart_event *evt, void *user_data)
{
	static struct uart_msg_queue_item new_message;

	switch (evt->type) {
		case UART_TX_DONE:
			// Free up the written bytes in the TX FIFO
			ring_buf_get_finish(&app_tx_fifo, bytes_claimed);

			// If there is more data in the TX fifo, start the transmission
			if(uart_tx_get_from_queue() == 0) {
				// Or release the semaphore if the TX fifo is empty
				k_sem_give(&tx_done);
			}
			break;
		
		case UART_RX_RDY:
			rx_received_since_enable = true;
			new_message.bytes = evt->data.rx.buf + evt->data.rx.offset;
			new_message.length = evt->data.rx.len;
			if(k_msgq_put(&uart_rx_msgq, &new_message, K_NO_WAIT) != 0){
				printk("Error: Uart RX message queue full!\n");
			}
			break;
		
		case UART_RX_BUF_REQUEST:
			if(free_rx_bytes >= UART_RX_DBL_BUF_SIZE) {
				UART_RX_BUF_INC();
				uart_rx_buf_rsp(dev_uart, UART_RX_NEXT_BUF, UART_RX_DBL_BUF_SIZE);
				free_rx_bytes -= UART_RX_DBL_BUF_SIZE;
			}
			break;

		case UART_RX_BUF_RELEASED:
			break;
		case UART_RX_STOPPED:
			break;
		case UART_RX_DISABLED:
			if(free_rx_bytes >= UART_RX_DBL_BUF_SIZE) {
				int used = k_msgq_num_used_get(&uart_rx_msgq);
				if(used > 0) {
					printk("RX DIS. Re-enabling.\n");
					if(rx_received_since_enable) {
						free_rx_bytes -= UART_RX_DBL_BUF_SIZE;
						UART_RX_BUF_INC();
					}
					int ret = uart_rx_enable(dev_uart, UART_RX_NEXT_BUF, UART_RX_DBL_BUF_SIZE, UART_RX_TIMEOUT_US);
					if(ret) {
						printk("UART rx enable error in disable callback: %d\n", ret);
						return;
					}
				} else {
					printk("RX DIS. Empty bufs Re-enabling.\n");
					free_rx_bytes = UART_RX_BUF_SIZE - UART_RX_DBL_BUF_SIZE;
					uart_rx_buf_index = 0;
					int ret = uart_rx_enable(dev_uart, UART_RX_NEXT_BUF, UART_RX_DBL_BUF_SIZE, UART_RX_TIMEOUT_US);
					if(ret) {
						printk("UART rx enable error in disable callback: %d\n", ret);
						return;
					}
				}
			} else {
				printk("RX dis, buf full. Set flag\n");
				rx_enable_request = true;
			}
			rx_received_since_enable = false;
			break;
		
		default:
			break;
	}
}

void app_uart_init(void)
{
	int ret;
	dev_uart = DEVICE_DT_GET(DT_NODELABEL(my_uart));
	if(!device_is_ready(dev_uart)) {
		printk("UART device not ready!\n");
		return;
	}

	ret = uart_callback_set(dev_uart, app_uart_async_callback, NULL);
	if(ret) {
		printk("Uart callback set error: %d\n", ret);
		return;
	}
	ret = uart_rx_enable(dev_uart, UART_RX_NEXT_BUF, UART_RX_DBL_BUF_SIZE, UART_RX_TIMEOUT_US);
	if(ret) {
		printk("UART rx enable error: %d\n", ret);
		return;
	}
	free_rx_bytes -= UART_RX_DBL_BUF_SIZE;
}

// Function to send UART data, by writing it to a ring buffer (FIFO) in the application
// WARNING: This function is not thread safe! If you want to call this function from multiple threads a semaphore should be used
int app_uart_send(const uint8_t * data_ptr, uint32_t data_len, k_timeout_t timeout)
{
	while(1) {
		// Try to move the data into the TX ring buffer
		uint32_t written_to_buf = ring_buf_put(&app_tx_fifo, data_ptr, data_len);
		data_len -= written_to_buf;
		
		// In case the UART TX is idle, start transmission
		if(k_sem_take(&tx_done, K_NO_WAIT) == 0) {
			uart_tx_get_from_queue();
		}	
		
		// In case all the data was written, exit the loop
		if(data_len == 0) break;

		// If timeout if 0, exit immediately
		if(timeout.ticks == 0) break;

		// In case some data is still to be written, sleep for some time and run the loop one more time
		k_msleep(10);
		data_ptr += written_to_buf;
	}

	return 0;
}

int app_uart_rx(uint8_t ** data_ptr, uint32_t * data_len, k_timeout_t timeout)
{
	int ret;
	struct uart_msg_queue_item incoming_message;

	// Check for a new message in the buffer
	ret = k_msgq_get(&uart_rx_msgq, &incoming_message, timeout);
	if(ret != 0) return ret;

	*data_ptr = incoming_message.bytes;
	*data_len = incoming_message.length;

	return 0;
}

int app_uart_rx_free(uint32_t bytes)
{
	free_rx_bytes += bytes;

	if(rx_enable_request && free_rx_bytes >= UART_RX_DBL_BUF_SIZE) {
		rx_enable_request = false;
		printk("Buffers freed. Re-en RX\n");
		free_rx_bytes -= UART_RX_DBL_BUF_SIZE;
		UART_RX_BUF_INC();
		int ret = uart_rx_enable(dev_uart, UART_RX_NEXT_BUF, UART_RX_DBL_BUF_SIZE, UART_RX_TIMEOUT_US);
		if(ret) {
			printk("UART rx enable error in app_uart_rx(): %d\n", ret);
			return ret;
		}
	}
	return 0;
}