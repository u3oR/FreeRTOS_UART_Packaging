#ifndef _DEVICE_UART_H
#define _DEVICE_UART_H

struct UART_Device 
{
	const char *name;
	struct UART_Data *prvdata;
	
	int (*Init)(	struct UART_Device *pDev, 
					int baud, 
					char parity, 
					int stop);
	
    int (*Send)(	struct UART_Device *pDev, 
					uint8_t *data, 
					int len, 
					int timeout_ms);
	
	int (*Recv)(	struct UART_Device *pDev, 
					uint8_t *data, 
					int len, 
					int timeout_ms);

};

#endif
