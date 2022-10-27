/****************************************************************************
 * Copyright (C) 2020 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file lab3-1_main.c
 * @author Fabrice Muller
 * @date 20 Oct. 2020
 * @brief File containing the lab3-1 of Part 3.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

static const char* TAG = "SEM";

/* Application constants */
#define STACK_SIZE     4096
#define TABLE_SIZE     400

/* Task Priority */
const uint32_t TIMER_TASK_PRIORITY = 5;
const uint32_t INC_TABLE_TASK_PRIORITY = 4;
const uint32_t DEC_TABLE_TASK_PRIORITY = 4;

/*  Creating  Binary  semaphore  */
TaskHandle_t  xhClk = NULL;
TaskHandle_t  xhInc = NULL;
TaskHandle_t  xhDec = NULL;

/* Communications */


/* Tasks */
void vTaskTimer(void *pvParameters);
void vTaskIncTable(void *pvParameters);
void vTaskDecTable(void *pvParameters);

/* Datas */
int Table[TABLE_SIZE];

/* Main function */
void app_main(void) {

	/* Init Table */
	memset(Table, 0, TABLE_SIZE*sizeof(int));

	/* Create semaphore */
	
	/* Using  semaphore  
	xSemaphoreGive(xSemClk);
	xSemaphoreTake(xSemClk , portMAX_DELAY);*/

	/* Stop scheduler */
	vTaskSuspendAll();

	/* Create Tasks */
	xTaskCreatePinnedToCore(vTaskTimer,	"Task TIMER", STACK_SIZE, (void*)"Task Timer", 5, &xhClk,CORE_0);					
	xTaskCreatePinnedToCore(vTaskIncTable,	"Task IncTable", STACK_SIZE, (void*)"Task IncTable", 3, &xhInc,CORE_0);					
	xTaskCreatePinnedToCore(vTaskDecTable,	"Task DecTable", STACK_SIZE, (void*)"Task DecTable", 4, &xhDec,CORE_0);					


	/* Continue scheduler */
	xTaskResumeAll();

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vTaskTimer(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for(;;){
		vTaskDelayUntil (&xLastWakeTime , pdMS_TO_TICKS (250));
		COMPUTE_IN_TIME_MS(20);
		DISPLAY("Task Timer : give notification");
		xTaskNotifyGive(xhInc);
		xTaskNotifyGive(xhInc);
		xTaskNotifyGive(xhDec);
	}

}

void vTaskIncTable(void *pvParameters) {
	int ActivationNumber = 0;
	int constNumber = *(int*)pvParameters;

	for(;;){
		if(ulTaskNotifyTake(pdFALSE,pdMS_TO_TICKS (5000))){
			DISPLAY("%d", ulTaskNotifyTake(pdFALSE,pdMS_TO_TICKS (5000)));
			if(ActivationNumber == 0){
				DISPLAY("Table inc : take notif");
				for(int i =0;i<TABLE_SIZE;i++){
					Table[i]+= constNumber;
				}
				COMPUTE_IN_TIME_MS(50);
				ActivationNumber = 4;
			}
			else{
				DISPLAY("Table inc ,,, Active#0");
				ActivationNumber -=1;
			}
		}
		
	}
}

void vTaskDecTable(void *pvParameters) {
	for(;;){
		if(ulTaskNotifyTake(pdFALSE,pdMS_TO_TICKS (5000))){
			DISPLAY("%d", ulTaskNotifyTake(pdFALSE,pdMS_TO_TICKS (5000)));
			DISPLAY("Table dec : Take notif");
			for(int i =0;i<TABLE_SIZE;i++){
				Table[i]-= 1;
			}
			COMPUTE_IN_TIME_MS(50);
		}
	}
		
}