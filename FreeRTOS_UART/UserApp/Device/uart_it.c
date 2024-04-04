#include "uart_it.h"

#include <string.h>
#include <stdbool.h>

#include "arm_math.h"
#include "main.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/************UART*************/

struct UART_Data
{
    void *uart_handle;
    SemaphoreHandle_t *xTxSem;
    QueueHandle_t *xRxQueue;
    uint8_t rxdata;
};

#define get_prvdata(pDev) ((struct UART_Data *)((pDev)->prvdata))


static int stm32_uart_init(struct UART_Device *pDev, int baud, char parity, int stop)
{
    get_prvdata(pDev)->xTxSem = xSemaphoreCreateBinary();
    get_prvdata(pDev)->xRxQueue = xQueueCreate(100, 1);
    
    // ������һ�����ݽ���
    HAL_UART_Receive_IT(get_prvdata(pDev)->uart_handle, 
                        &(get_prvdata(pDev)->rxdata), 1);
    
    return 0;
}

static int stm32_uart_send(struct UART_Device *pDev, uint8_t *data, int len, int timeout_ms)
{
    // �����жϷ�������
    HAL_UART_Transmit_IT(pDev->prvdata, data, len);
    // �ȴ��ź���
    if(pdTRUE != xSemaphoreTake(*(get_prvdata(pDev)->xTxSem), timeout_ms)){
        return -1;
    }
    return 0;
}

static int stm32_uart_recv(struct UART_Device *pDev, uint8_t *data, int len, int timeout_ms)
{
    if (pdPASS != xQueueReceive(*(get_prvdata(pDev)->xRxQueue), data, timeout_ms)) {
        return -1;
    }
    return 0;
}

static struct UART_Data stm32_uart1_data = {0};

static struct UART_Device stm32_uart1 = {
    .name = "stm32_uart1",
    .prvdata = &stm32_uart1_data,
    .Init = stm32_uart_init,
    .Send = stm32_uart_send,
    .Recv = stm32_uart_recv
};

struct UART_Device *stm32_uart_devs[] = {&stm32_uart1, };

struct UART_Device *GetUARTDevie(const char *name)
{
    int stm32_uart_devs_len = sizeof(stm32_uart_devs)/sizeof(stm32_uart_devs[0]);

    for (int i = 0; i < stm32_uart_devs_len; i++)
    {
        if(strcmp(stm32_uart_devs[i]->name, name) == 0)
            return stm32_uart_devs[i];
    }

    return NULL;
}


/**
 * @brief ������ɻص�����
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        // �ͷ��ź���
        xSemaphoreGiveFromISR(get_prvdata(&stm32_uart1)->xTxSem, NULL);
    }
}

/**
 * @brief ������ɻص�����
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct UART_Data *uart_data;

    if (huart == &huart1)
    {
        uart_data = get_prvdata(&stm32_uart1);

        // д�����
        xQueueSendFromISR(*uart_data->xRxQueue, &(uart_data->rxdata), NULL);
        // �ٴ��������ݽ���
        HAL_UART_Receive_IT(&huart1, &(uart_data->rxdata), 1);
    }
}



#undef get_prvdata