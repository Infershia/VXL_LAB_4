/*
 * task.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Gia Minh
 */

#include "main.h"
#include "task.h"
#include "scheduler.h"

// Define the task list here
taskList SCH_TASK_LIST;

// Global variable to generate unique Task IDs
static uint32_t Next_Task_ID = 1;

void SCH_Init() {
	SCH_TASK_LIST.numofTask = 0;
	Next_Task_ID = 1;

	for(int index = 0; index < SCH_MAX_TASKS; index++) {
		SCH_TASK_LIST.TASK_QUEUE[index].Delay = 0;
		SCH_TASK_LIST.TASK_QUEUE[index].Period = 0;
		SCH_TASK_LIST.TASK_QUEUE[index].RunMe = 0;
		SCH_TASK_LIST.TASK_QUEUE[index].pTask = 0;
		SCH_TASK_LIST.TASK_QUEUE[index].TaskID = NO_TASK_ID;
	}
}

static void insert_to_list(sTask task) {
	if (SCH_TASK_LIST.numofTask == 0) {
		SCH_TASK_LIST.TASK_QUEUE[0] = task;
		SCH_TASK_LIST.numofTask += 1;
		return;
	} else {
		int pos = 0;
		int prev = SCH_TASK_LIST.TASK_QUEUE[0].Delay;
		int sum = SCH_TASK_LIST.TASK_QUEUE[0].Delay;

		while (pos < SCH_TASK_LIST.numofTask && task.Delay > sum) {
			pos++;
			if (pos < SCH_TASK_LIST.numofTask) {
				prev = sum;
				sum += SCH_TASK_LIST.TASK_QUEUE[pos].Delay;
			}
		}

		// Add to rear
		if (pos == SCH_TASK_LIST.numofTask) {
			task.Delay -= sum;
			SCH_TASK_LIST.TASK_QUEUE[pos] = task;
			SCH_TASK_LIST.numofTask += 1;
			return;
		}
		// Add to front or middle
		else {
			// Shift tasks to make room
			for (int i = SCH_TASK_LIST.numofTask; i > pos; i--) {
				SCH_TASK_LIST.TASK_QUEUE[i] = SCH_TASK_LIST.TASK_QUEUE[i - 1];
			}

			// Add to front
			if (pos == 0) {
				SCH_TASK_LIST.TASK_QUEUE[pos] = task;
				sum = 0;
				prev = task.Delay;
			}
			// Add to middle
			else {
				task.Delay -= prev;
				sum = prev;
				prev += task.Delay;
				SCH_TASK_LIST.TASK_QUEUE[pos] = task;
			}

			// Update delay of next task (index[POS+1])
			sum += SCH_TASK_LIST.TASK_QUEUE[pos + 1].Delay;
			SCH_TASK_LIST.TASK_QUEUE[pos + 1].Delay = sum - prev;
			SCH_TASK_LIST.numofTask += 1;
		}
	}
}

uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t delay, uint32_t period) {
	if (SCH_TASK_LIST.numofTask >= SCH_MAX_TASKS) {
		return NO_TASK_ID;
	}

	sTask temp;
	temp.pTask = pFunction;
	temp.Delay = delay / TICK;
	temp.Period = period / TICK;
	temp.RunMe = 0;
	temp.TaskID = Next_Task_ID++;

	insert_to_list(temp);

	return temp.TaskID;
}

void SCH_Update() {
	for (int i = 0; i < SCH_TASK_LIST.numofTask; i++) {
		if (SCH_TASK_LIST.TASK_QUEUE[i].pTask) {
			if (SCH_TASK_LIST.TASK_QUEUE[i].Delay == 0) {
				SCH_TASK_LIST.TASK_QUEUE[i].RunMe += 1;
				if (SCH_TASK_LIST.TASK_QUEUE[i].Period) {
					SCH_TASK_LIST.TASK_QUEUE[i].Delay = SCH_TASK_LIST.TASK_QUEUE[i].Period;
				}
			} else {
				SCH_TASK_LIST.TASK_QUEUE[i].Delay -= 1;
			}
		}
	}
}

static void delete_task_at_index(int index) {
	int add_back_flag = 0;
	sTask temp;

	if (SCH_TASK_LIST.TASK_QUEUE[index].Period) {
		add_back_flag = 1;
		temp = SCH_TASK_LIST.TASK_QUEUE[index];
	}

	// Update delay of next task
	if (index < SCH_TASK_LIST.numofTask - 1) {
		SCH_TASK_LIST.TASK_QUEUE[index + 1].Delay += SCH_TASK_LIST.TASK_QUEUE[index].Delay;
	}

	// Shift left all tasks
	for (int i = index; i < SCH_TASK_LIST.numofTask - 1; i++) {
		SCH_TASK_LIST.TASK_QUEUE[i] = SCH_TASK_LIST.TASK_QUEUE[i + 1];
	}

	// Clear last element
	int last = SCH_TASK_LIST.numofTask - 1;
	SCH_TASK_LIST.TASK_QUEUE[last].pTask = 0x0000;
	SCH_TASK_LIST.TASK_QUEUE[last].Delay = 0;
	SCH_TASK_LIST.TASK_QUEUE[last].Period = 0;
	SCH_TASK_LIST.TASK_QUEUE[last].RunMe = 0;
	SCH_TASK_LIST.TASK_QUEUE[last].TaskID = NO_TASK_ID;

	SCH_TASK_LIST.numofTask -= 1;

	if (add_back_flag == 1) {
		insert_to_list(temp);
	}
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
	for (int index = 0; index < SCH_TASK_LIST.numofTask; index++) {
		if (SCH_TASK_LIST.TASK_QUEUE[index].TaskID == taskID) {
			delete_task_at_index(index);
			return 1;
		}
	}

	return 0;
}

void SCH_Dispatch_Tasks() {
	int i = 0;
	while (i < SCH_TASK_LIST.numofTask) {
		if (SCH_TASK_LIST.TASK_QUEUE[i].RunMe > 0) {
			// Run the task
			(*SCH_TASK_LIST.TASK_QUEUE[i].pTask)();

			// Reset RunMe flag
			SCH_TASK_LIST.TASK_QUEUE[i].RunMe -= 1;

			// If one-shot, delete it
			if (SCH_TASK_LIST.TASK_QUEUE[i].Period == 0) {
				delete_task_at_index(i);
				// Don't increment i
			} else {
				i++; // Move to next task
			}
		} else {
			i++; // Move to next task
		}
	}
}
