/*
 * task.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Gia Minh
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_MAX_TASKS	40
#define TICK			10
#define NO_TASK_ID		0

typedef struct {
	void (*pTask)(void);  // Pointer to the task function
	uint32_t Delay;       // Delay (ticks) until the function will (next) be run
	uint32_t Period;      // Interval (ticks) between subsequent runs
	uint8_t RunMe;        // Incremented when task is due to execute
	uint32_t TaskID;
} sTask;

typedef struct {
	sTask TASK_QUEUE[SCH_MAX_TASKS];
	int numofTask;
} taskList;

// Task list storage
extern taskList SCH_TASK_LIST;

// Function
void SCH_Init(void);
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
uint8_t SCH_Delete_Task(uint32_t taskID);

#endif /* INC_SCHEDULER_H_ */
