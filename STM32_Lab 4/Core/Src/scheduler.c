/*
 * task.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Gia Minh
 */

#include "main.h"
#include "task.h"
#include "scheduler.h"

taskList SCH_TASK_LIST;
static uint32_t newTaskID = 1;

static uint32_t Get_New_Task_ID(void) {
	newTaskID++;
		if (newTaskID == NO_TASK_ID) newTaskID++;
			return newTaskID;
}

void SCH_Init(void) {
	SCH_TASK_LIST.numofTask = 0;
		for (int i = 0; i < SCH_MAX_TASKS; i++) {
			SCH_TASK_LIST.TASK_QUEUE[i].pTask = 0;
			SCH_TASK_LIST.TASK_QUEUE[i].Delay = 0;
			SCH_TASK_LIST.TASK_QUEUE[i].Period = 0;
			SCH_TASK_LIST.TASK_QUEUE[i].RunMe = 0;
			SCH_TASK_LIST.TASK_QUEUE[i].TaskID = NO_TASK_ID;
		}
}

void SCH_Add_Task(void (*pFunction)(), uint32_t delay_ms, uint32_t period_ms) {
	uint32_t delay = delay_ms / TICK;
	uint32_t period = period_ms / TICK;
	uint32_t sumDelay = 0;
	uint32_t newPos = 0;


	for (newPos = 0; newPos < SCH_TASK_LIST.numofTask; newPos++) {
		sumDelay += SCH_TASK_LIST.TASK_QUEUE[newPos].Delay;

		if (sumDelay > delay) {
			uint32_t newDelay = delay - (sumDelay - SCH_TASK_LIST.TASK_QUEUE[newPos].Delay);
			SCH_TASK_LIST.TASK_QUEUE[newPos].Delay = sumDelay - delay;

			for (int j = SCH_TASK_LIST.numofTask; j > newPos; j--) {
				SCH_TASK_LIST.TASK_QUEUE[j] = SCH_TASK_LIST.TASK_QUEUE[j - 1];
			}


			SCH_TASK_LIST.TASK_QUEUE[newPos].pTask = pFunction;
			SCH_TASK_LIST.TASK_QUEUE[newPos].Delay = newDelay;
			SCH_TASK_LIST.TASK_QUEUE[newPos].Period = period;
			SCH_TASK_LIST.TASK_QUEUE[newPos].RunMe = (newDelay == 0);
			SCH_TASK_LIST.TASK_QUEUE[newPos].TaskID = Get_New_Task_ID();
			SCH_TASK_LIST.numofTask++;
			return;
		}
	}

	SCH_TASK_LIST.TASK_QUEUE[newPos].pTask = pFunction;
	SCH_TASK_LIST.TASK_QUEUE[newPos].Delay = delay - sumDelay;
	SCH_TASK_LIST.TASK_QUEUE[newPos].Period = period;
	SCH_TASK_LIST.TASK_QUEUE[newPos].RunMe = (delay == sumDelay);
	SCH_TASK_LIST.TASK_QUEUE[newPos].TaskID = Get_New_Task_ID();
	SCH_TASK_LIST.numofTask++;
}

void SCH_Update(void) {
	if (SCH_TASK_LIST.numofTask == 0) return;

	if (SCH_TASK_LIST.TASK_QUEUE[0].Delay > 0) {
			SCH_TASK_LIST.TASK_QUEUE[0].Delay--;
		}

	if (SCH_TASK_LIST.TASK_QUEUE[0].Delay == 0) {
		SCH_TASK_LIST.TASK_QUEUE[0].RunMe = 1;
		}
}

static void delete_task_at_index(int index) {
		if (index < SCH_TASK_LIST.numofTask - 1) {
			SCH_TASK_LIST.TASK_QUEUE[index + 1].Delay += SCH_TASK_LIST.TASK_QUEUE[index].Delay;
		  }

		for (int i = index; i < SCH_TASK_LIST.numofTask - 1; i++) {
		SCH_TASK_LIST.TASK_QUEUE[i] = SCH_TASK_LIST.TASK_QUEUE[i + 1];
		   }

		int last = SCH_TASK_LIST.numofTask - 1;
		SCH_TASK_LIST.TASK_QUEUE[last].pTask = 0;
		SCH_TASK_LIST.TASK_QUEUE[last].Delay = 0;
		SCH_TASK_LIST.TASK_QUEUE[last].Period = 0;
		SCH_TASK_LIST.TASK_QUEUE[last].RunMe = 0;
		SCH_TASK_LIST.TASK_QUEUE[last].TaskID = NO_TASK_ID;
		SCH_TASK_LIST.numofTask--;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
		for (int i = 0; i < SCH_TASK_LIST.numofTask; i++) {
				if (SCH_TASK_LIST.TASK_QUEUE[i].TaskID == taskID) {
					delete_task_at_index(i);
					return 1;
				}
			}
		return 0;
}

void SCH_Dispatch_Tasks(void) {
	if (SCH_TASK_LIST.numofTask == 0) return;

	if (SCH_TASK_LIST.TASK_QUEUE[0].RunMe > 0) {
		SCH_TASK_LIST.TASK_QUEUE[0].RunMe = 0;
		sTask temp = SCH_TASK_LIST.TASK_QUEUE[0];
		(*temp.pTask)();

		delete_task_at_index(0);

		if (temp.Period != 0) {
			SCH_Add_Task(temp.pTask, temp.Period * TICK, temp.Period * TICK);
		}
	}
}
