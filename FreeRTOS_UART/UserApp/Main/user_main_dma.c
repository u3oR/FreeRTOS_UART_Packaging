#include "user_main.h"
#include "FreeRTOSConfig.h"
#include "main.h"
#include "usart.h"
#include "string.h"
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "arm_math.h"


void vTask1(void *argument);
void vTask2(void *argument);

void user_main(void)
{
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    
    xTaskCreate(vTask1, "vTask1", 128, NULL, 1, NULL);
    xTaskCreate(vTask2, "vTask2", 128, NULL, 1, NULL);

    vTaskStartScheduler();
    
	Error_Handler();
}

__NO_RETURN void vTask1(void *argument)
{
	(void)argument;
    
    while(1){
        HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
        vTaskDelay(500);
    }
}

__NO_RETURN void vTask2(void *argument)
{
	(void)argument;
    
    while(1){
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(500);
    }
}


