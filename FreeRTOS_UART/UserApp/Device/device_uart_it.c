/*********************************
 * Included Header Files
 *********************************/
#include "device_uart.h"

#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
/*********************************
 * Macro Definitions
 *********************************/

/*********************************
 * Structure Definitions
 *********************************/

struct UART_Data
{
    UART_HandleTypeDef *uart_handle;
    SemaphoreHandle_t xTxSem;
    QueueHandle_t xRxQueue;
    uint8_t rxdata;
};

/*********************************
 * Private Function Declarations
 *********************************/

static int stm32_uart_init(struct UART_Device *pDev, int baud, char parity, int stop);
static int stm32_uart_send(struct UART_Device *pDev, uint8_t *data, int len, int timeout_ms);
static int stm32_uart_recv(struct UART_Device *pDev, uint8_t *data, int timeout_ms);

/*********************************
 * Global Variables
 *********************************/

static struct UART_Device *g_cur_uart1_dev;

static struct UART_Data __stm32_uart1_data = {
    .uart_handle = &huart1
};

static struct UART_Device stm32_uart1 = {
    .name = "stm32_uart1",
    .prvdata = &__stm32_uart1_data,
    .Init = stm32_uart_init,
    .Send = stm32_uart_send,
    .Recv = stm32_uart_recv
};

static struct UART_Device *stm32_uart_devs[] = {
    &stm32_uart1, 
};

/*********************************
 * External Function Implementations
 *********************************/
/**
 * @brief Send completion callback function
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct UART_Data *uart_data;
    
    if (huart == &huart1)
    {
        uart_data = (&stm32_uart1)->prvdata;
        
        xSemaphoreGiveFromISR(uart_data->xTxSem, NULL);
    }
}

/**
 * @brief Receive completion callback function
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct UART_Data *uart_data;

    if (huart == &huart1)
    {
        uart_data = (&stm32_uart1)->prvdata;

        // Write to queue
        xQueueSendFromISR(uart_data->xRxQueue, &(uart_data->rxdata), NULL);
        // Restart data reception
        HAL_UART_Receive_IT(&huart1, &(uart_data->rxdata), 1);
    }
}


/*********************************
 * Private Function Implementations
 *********************************/
static int stm32_uart_init(struct UART_Device *pDev, int baud, char parity, int stop)
{
    struct UART_Data *uart_data = pDev->prvdata;
    
    g_cur_uart1_dev = pDev;
    
    uart_data->xTxSem = xSemaphoreCreateBinary();
    uart_data->xRxQueue = xQueueCreate(100, 1);
    
    // Start the first data reception
    HAL_UART_Receive_IT(uart_data->uart_handle, &uart_data->rxdata, 1);
    
    return 0;
}

static int stm32_uart_send(struct UART_Device *pDev, uint8_t *data, int len, int timeout_ms)
{
    struct UART_Data *uart_data = pDev->prvdata;
    
    // Trigger interrupt to send data
    HAL_UART_Transmit_IT(pDev->prvdata, data, len);
    
    if(pdTRUE != xSemaphoreTake(uart_data->xTxSem, timeout_ms)){
        return -1;
    }
    return 0;
}

static int stm32_uart_recv(struct UART_Device *pDev, uint8_t *data, int timeout_ms)
{
    struct UART_Data *uart_data = pDev->prvdata;
    
    if (pdPASS != xQueueReceive(uart_data->xRxQueue, data, timeout_ms)) {
        return -1;
    }
    return 0;
}
/*********************************
 * Public Function Implementations
 *********************************/

struct UART_Device *DEV_UART_GetDevice(const char *name)
{
    int stm32_uart_devs_len = sizeof(stm32_uart_devs)/sizeof(stm32_uart_devs[0]);

    for (int i = 0; i < stm32_uart_devs_len; i++)
    {
        if(strcmp(stm32_uart_devs[i]->name, name) == 0)
            return stm32_uart_devs[i];
    }

    return NULL;
}


