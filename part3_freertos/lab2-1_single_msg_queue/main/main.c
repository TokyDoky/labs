#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

/**************************************/
/**************************************/
/* Scenarios                          */

#define DIFF_PRIORITY

#define PINNED_TO_CORE 0x00
//	 0x00: Task 1: Core 0, Task 2: Core 0
//   0x01: Task 1: Core 0, Task 2: Core 1
//   0x10: Task 1: Core 1, Task 2: Core 0
//   0x11: Task 1: Core 1, Task 2: Core 1, Task 3: Core 0

//#define IDLE_HOOKS 

#define TASK_DELAY

#define PERIODIC_TASK_DELAY

/**************************************/
/**************************************/

/* Default stack size for tasks */
static const uint32_t STACK_SIZE = 4000;

#ifdef DIFF_PRIORITY

static const uint32_t T1_PRIO = 2;
static const uint32_t T2_PRIO = 2;
static const uint32_t T3_PRIO = 3;

#else

static const uint32_t T1_PRIO = 5;
static const uint32_t T2_PRIO = 5;


#endif

#ifdef IDLE_HOOKS

volatile uint32_t countIdle0 = 0;
volatile uint32_t countIdle1 = 0;

bool vApplicationIdleHook_0 ( void ){
	countIdle0++;
	return true;
}

bool vApplicationIdleHook_1 ( void ){
	countIdle1++;
	return true;
}

#ifndef PERIODIC_TASK_DELAY
/* Buffer to extract trace information */
static char buffer[40*12];
#endif

#endif

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		(0x7FFFF)

/* The task function. */
void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);

QueueHandle_t xQueue1 = NULL;
QueueSetHandle_t xQueueSet;

void app_main(void) {

	xQueue1 = xQueueCreate(1, sizeof(uint32_t));
	DISPLAY("Start of app_main task, priority = %d",uxTaskPriorityGet(NULL));

#ifdef IDLE_HOOKS
      esp_register_freertos_idle_hook_for_cpu(vApplicationIdleHook_0, CORE_0);
      esp_register_freertos_idle_hook_for_cpu(vApplicationIdleHook_1, CORE_1);
#endif

	vTaskSuspendAll();

	/* Create tasks. */
	#if PINNED_TO_CORE == 0x00
		xTaskCreatePinnedToCore(Task1, "Task 1", STACK_SIZE, (void*)"Task 1", T1_PRIO,	NULL,CORE_1);					
		xTaskCreatePinnedToCore(Task2, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL,CORE_0);
		xTaskCreatePinnedToCore(Task3, "Task 3", STACK_SIZE, (void*)"Task 3", T3_PRIO, NULL,CORE_0);
	#elif PINNED_TO_CORE == 0x01
		xTaskCreatePinnedToCore(vTaskFunction,	"Task 1", STACK_SIZE,	(void*)"Task 1", T1_PRIO,	NULL,CORE_0);					
		xTaskCreatePinnedToCore(vTaskFunction, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL,CORE_1);
	#elif PINNED_TO_CORE == 0x10
		xTaskCreatePinnedToCore(vTaskFunction,	"Task 1", STACK_SIZE,	(void*)"Task 1", T1_PRIO,	NULL,CORE_1);					
		xTaskCreatePinnedToCore(vTaskFunction, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL,CORE_0);	
	#elif PINNED_TO_CORE == 0x11
		xTaskCreatePinnedToCore(vTaskFunction,	"Task 1", STACK_SIZE,	(void*)"Task 1", T1_PRIO,	NULL,CORE_1);					
		xTaskCreatePinnedToCore(vTaskFunction, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL,CORE_1);			
		xTaskCreatePinnedToCore(vTaskFunction, "Task 3", STACK_SIZE, (void*)"Task 3", T3_PRIO, NULL, CORE_0);
	#else
		xTaskCreate(vTaskFunction,	"Task 1", STACK_SIZE,	(void*)"Task 1", T1_PRIO,	NULL);					
		xTaskCreate(vTaskFunction, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL);
		
	#endif
	xTaskResumeAll();



#ifdef IDLE_HOOKS

	#ifndef PERIODIC_TASK_DELAY
		/* Print task list */
		vTaskList(buffer);
		DISPLAY("Trace information");
		printf("--------------------------------------------------------\n");
		printf("task\t\tstate\tprio\tstack\ttsk id\tcore id\n");
		printf("--------------------------------------------------------\n");
		printf(buffer);
	#endif

	for (;;) {
		vTaskDelay(100 / portTICK_PERIOD_MS);    // 100 ms
		DISPLAY("IDLE0: %d, IDLE1:%d", countIdle0, countIdle1);
	}
#else
	for (;;) {
		vTaskDelay(100 / portTICK_PERIOD_MS);    // 100 ms
		//DISPLAY("Hello app_main task!");
	}

#endif

	/* to ensure its exit is clean */
	vTaskDelete(NULL);	
}
/*-----------------------------------------------------------*/

// void vTaskFunction(void *pvParameters) {
// 	char *pcTaskName;
// 	volatile uint32_t ul;

// 	TickType_t xLastWakeTime;
// 	xLastWakeTime = xTaskGetTickCount();

// 	/* The string to print out is passed in via the parameter.  Cast this to a
// 	character pointer. */
// 	pcTaskName = (char *)pvParameters;

// 	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

// 	/* As per most tasks, this task is implemented in an infinite loop. */
// 	for (;; ) {
// 		DISPLAY("Run computation of %s", pcTaskName);

// 		/* Delay for simulating a computation */
// 		for (ul = 0; ul < mainDELAY_LOOP_COUNT; ul++){		
// 		}

// 		// To run APP_MAIN TASK
// 		#ifdef TASK_DELAY
// 			// DISPLAY("Delay of %s", pcTaskName);
// 			// vTaskDelay(400 / portTICK_PERIOD_MS);
// 			#ifdef PERIODIC_TASK_DELAY
// 				DISPLAY("Periodic  Delay of %s", pcTaskName);
// 				vTaskDelayUntil (&xLastWakeTime , pdMS_TO_TICKS (300));
// 			#else
// 				DISPLAY("Approximated  Delay  of %s", pcTaskName);
// 				vTaskDelay(pdMS_TO_TICKS (300));
// 			#endif
// 		#endif
// 	}
// }

void Task1(void *pvParameters){
	uint32_t result;
	uint32_t lSentValue = 50;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for(;; ){
		vTaskDelay(pdMS_TO_TICKS (100));
		DISPLAY("Enqueuing %d", lSentValue);
		result = xQueueSend(xQueue1, &lSentValue, 0);
		COMPUTE_IN_TIME_MS(40);
		// block  periodically : 500ms
		vTaskDelayUntil (&xLastWakeTime , pdMS_TO_TICKS (500));
	}
}

void Task2(void *pvParameters){
	int32_t lReceivedValue;

	for(;; ){
		xQueueReceive(xQueue1, &lReceivedValue, portMAX_DELAY);
		// display  task  number  and  message
		DISPLAY ("What received Task 2 :  %d", lReceivedValue);
		//  Compute  time : 30 ms
		COMPUTE_IN_TIME_MS (30);
	}
}

void Task3(void *pvParameters){
	int32_t lSentValue;
	for(;; ){
		DISPLAY("Delay of Task 3");
		vTaskDelay(pdMS_TO_TICKS (100));
		DISPLAY("Block end");
		COMPUTE_IN_TIME_MS(20);
	}
}
