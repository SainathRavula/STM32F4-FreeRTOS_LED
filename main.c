#include "stm32f4xx.h" 
//contains definitions for all peripherals, registers, and other hardware-related constants.
//Uncomment the target board from stm32f4xx.h file

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "stm32f4xx_gpio.h" // Include GPIO library

#define CCM_RAM __attribute__((section(".ccmram")))
/* 
* Macro to use CCM (Core Coupled Memory) in STM32F4
* Defines a macro to place specific variables and functions in the Core Coupled Memory (CCM) of the STM32F4 microcontroller.
* __attribute__((section(".ccmram"))): This is an attribute specifier provided by GCC. Used to specify that the variable or function
 declared with this attribute should be placed in a specific section of memory during the linking process.
* It is a special type of memory;CCM is designed to provide fast, low-latency access to memory for critical system tasks, 
particularly those related to real-time processing.
* Other memory regions:- Flash memory, RAM, External Memory Interfaces(EMI), Pheripheral Memory regions
*/

#define TASK1_STACK_SIZE 50
StackType_t Task1Stack[TASK1_STACK_SIZE] CCM_RAM;
// Declares an array named Task1Stack of type StackType_t  
// Put task stack in CCM

StaticTask_t Task1Buffer CCM_RAM;  
// Put TCB in CCM

#define TASK2_STACK_SIZE 50
StackType_t Task2Stack[TASK2_STACK_SIZE] CCM_RAM;
// Declares an array named fpuTaskStack of type StackType_t  
// Put task stack in CCM

StaticTask_t Task2Buffer CCM_RAM;  
// Put TCB in CCM

void init_GPIO(void);
void vTask1_Red(void *pvParameters);
void vTask2_Green(void *pvParameters);

//=========================================================Main Function===================================================================
int main(void) 
{
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
/*
This is a function provided by the CMSIS library.It is used to  configure the priority grouping of the NVIC.
  
NVIC_PriorityGroup_4 indicates that the priority grouping will be configured with 4 bits for preemption priority and 0 bits for subpriority.
*/

      	init_GPIO();
 	xTaskCreateStatic(vTask1_Red, "RED LED", TASK1_STACK_SIZE, NULL, 1, Task1Stack, &Task1Buffer);
	xTaskCreateStatic(vTask2_Green, "Green LED", TASK2_STACK_SIZE, NULL, 1, Task2Stack, &Task2Buffer);
/*
xTaskCreateStatic(task_func, User_def_name, StackSize, Parameters, priority, Pointer_to_stack, pointer_to_TCB)
   
If a task is created using xTaskCreate() then the required RAM is automatically allocated from the FreeRTOS heap. 
If a task is created using xTaskCreateStatic() then the RAM is provided by the application writer,
  
Stack and TCB are placed in CCM of STM32F4.The CCM block is connected directly to the core, which leads to zero wait states
*/  

	vTaskStartScheduler();
	for(;;);
}
//=========================================================================================================================================



//=======================================================Configs===========================================================================

void vApplicationTickHook(void) {
}
/*
The purpose of vApplicationTickHook() is to allow user-defined code to be executed at each tick of the RTOS tick timer. 
You can use this hook function to perform periodic tasks, monitor system activity, or synchronize with external events.
*/



void vApplicationMallocFailedHook(void) {
  taskDISABLE_INTERRUPTS();
  //This prevents any interrupt service routines (ISRs) from  running while the system is in an unstable state due to the memory allocation failure.
   printf("[ERROR] Memory allocation failed! Halting program.\n");
  for(;;);
}
/*
vApplicationMallocFailedHook() will only be called if configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.
It is a hook function that will get called if a call to pvPortMalloc() fails.

pvPortMalloc() is called internally by the kernel whenever a task, queue,timer or semaphore is created. It is also called by various parts of the demo application.
If heap_1.c or heap_2.c are used, then the size of the heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in 
FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used to 
query the size of free heap space that remains (although it does not provide information on how the remaining heap might be fragmented). 
*/



void vApplicationIdleHook(void) 
{

}
/* 
vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h. 

It will be called on each iteration of the idle task. It is essential that code added to this hook function never attempts to block in any way 
(for example, call xQueueReceive() with a block time specified, or call vTaskDelay()). 
 
If the application makes use of the vTaskDelete() API function (as this demo application does) then it is also important that 
vApplicationIdleHook() is permitted to return to its calling function, 
because it is the responsibility of the idle task to clean up memory allocated by the kernel to any task that has since been deleted. 
*/
 
 
   
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
  (void) pcTaskName;
  (void) pxTask;
  //Run time stack overflow checking is performed if configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.
  //This hook function is called if a stack overflow is detected. 
  
  taskDISABLE_INTERRUPTS();
  for(;;);
}



StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;
/*
Used to provide the memory that is used by the Idle task. 

Two variables are being declared with attributes to place them in the Core Coupled Memory (CCM)
*/
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {

//ppxIdleTaskTCBBuffer: This parameter is a pointer to a pointer to a StaticTask_t structure.Similarly ppxIdleTaskStackBuffer
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
//Pass out a pointer to the StaticTask_t structure in which the Idle task's state will be stored.
  
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
//Pass out the array that will be used as the Idle task's stack.
 
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
 
//Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
//Note that, as the array is necessarily of type StackType_t, configMINIMAL_STACK_SIZE is specified in words, not bytes. 

}
/* 
configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an implementation of 
vApplicationGetIdleTaskMemory() to provide the memory that is used by the Idle task. 
*/



static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;
/*
used for storing the Task Control Block (TCB) and stack for the Timer service task, respectively.
They are declared with the static keyword, which limits their scope to the current file.
*/
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {

//ppxTimerTaskTCBBuffer: This parameter is a pointer to a pointer to a StaticTask_t structure.Similarly ppxTimerTaskStackBuffer
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/* 
configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the application must provide an implementation of 
vApplicationGetTimerTaskMemory() to provide the memory that is used by the Timer service task. 
*/

//=========================================================================================================================================





void init_GPIO(void) 
{
	/*
	RCC->AHB1ENR |= 0x00000008;
	GPIOD->MODER = 0x55000000;
	GPIOD->OSPEEDR |= 0xFF000000;
	*/
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
 	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
 	GPIO_Init(GPIOD, &GPIO_InitStruct);

}


void vTask1_Red(void *pvParameters) 
{
	
	for(;;)
	{
		printf("Red LED\n");
		GPIO_ToggleBits(GPIOD,GPIO_Pin_14);
		vTaskDelay(2000);
	}
}
void vTask2_Green(void *pvParameters) 
{
	for(;;)
	{
		printf("Green LED\n");
		GPIO_ToggleBits(GPIOD,GPIO_Pin_12);
		vTaskDelay(5000);
	}
}

