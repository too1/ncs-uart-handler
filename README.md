.. _uart-count-rx:

NCS-uart-handler
################

Overview
********
This example implements a UART async library that simplifies the interaction with the UART in async mode. 
The library handles the buffering of TX and RX data without relying on dynamic memory allocation, and with a minimal use of memcpy. 

Usage
*****

The library expects to find a UART interface with the nodelabel my_uart in your device tree, and will not build otherwise. 
In order to apply this nodelabel to one of the existing UART's, simply add the following to your overlay file (using uart0 as an example):

```c

my_uart: &uart0 {
};

```

The library starts an internal thread that will forward UART events, such as RX data and error conditions, to the application. 
In order to process these events an event handler must be implemented:

```c
static void on_app_uart_event(struct app_uart_evt_t *evt)
{
	switch(evt->type) {
		case APP_UART_EVT_RX:
      // Print the incoming data to the console
			printk("RX ");
			for(int i = 0; i < evt->data.rx.length; i++) printk("%.2x ", evt->data.rx.bytes[i]);
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
```
To initialize the library call the init function, and provide a pointer to your event handler:

```c
err = app_uart_init(on_app_uart_event);
if(err != 0) {
	printk("app_uart_init failed: %i\n", err);
}
```

To send UART data use the app_uart_send(..) function. A timeout parameter can be provided to time out after a predetermined point in time. 

```c
uint8_t my_data[] = {1,2,3,4,5};
app_uart_send(my_data, sizeof(my_data), K_FOREVER);
```

TODO
****
- Reliability testing
- Throughput testing
- Improved TX handling
- Making the library into a proper Zephyr library, including Kconfig parameters etc
