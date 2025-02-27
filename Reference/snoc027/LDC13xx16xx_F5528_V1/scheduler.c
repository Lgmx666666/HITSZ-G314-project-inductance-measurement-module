/*
 * scheduler.c
 *
 *  Created on: Dec 17, 2013
 *      Author: a0273922
 */

#include <msp430.h>
#include <stdint.h>

#include "scheduler.h"

// used in timer
volatile uint32_t timer_ticks;

// tasks
static scheduled_task_t * task_head = NULL;
static scheduled_task_t * task_tail = NULL;
static scheduled_task_t * task_ptr = NULL;

// interrupt-only statics
static scheduled_task_t * task_eptr = NULL;

void initScheduler() {
	SCHEDULER_CCR0 = SCHEDULER_DIV-1;                        // SMCLK divider
//  SCHEDULER_EX0 = TAIDEX_3;                              // Divide clock (selected below) further by 8
	SCHEDULER_CTL = TASSEL_2 + MC_1 + TACLR; //+ ID_2;     // SMCLK, upmode, clear TAR
	SCHEDULER_CCTL0 |= CCIE; // enable continuous sampling timer interrupt (TA0CCR0)
}

void executeTasks() {
	task_ptr = task_head;
	while (task_ptr != NULL) {
		if (task_ptr->timedout == TRUE) {
			(* task_ptr->function)(task_ptr);
			task_ptr->timedout = FALSE;
		}
		task_ptr = task_ptr->next;
	}
}

void addTask(scheduled_task_t * task) {
	// set timeout
	task->timeout = task->ticks + timer_ticks;
	task->active = FALSE;
	if (task_head == NULL) {
		task->prev = NULL;
		task->next = NULL;
		task_tail = task;
		task_head = task; // update task_head last to avoid conflicts with interrupt
	}
	else {
		task->prev = task_tail;
		task->next = NULL;
		task_tail->next = task;
		task_tail = task;
	}
	task->timedout = FALSE;
	task->active = TRUE;
}
void removeTask(scheduled_task_t * task) {
	if (task == NULL) return;
	if (task->next != NULL) {
		task_ptr = task->next;
		task_ptr->prev = task->prev;
	}
	else {
		task_tail = task->prev;
	}
	if (task->prev != NULL) {
		task_ptr = task->prev;
		task_ptr->next = task->next;
	}
	else {
		task_head = task->next;
	}
	task->timedout = FALSE;
	task->active = FALSE;
}
void removeAllTasks() {
	task_ptr = task_head;
	while (task_ptr != NULL) {
		task_ptr->active = FALSE;
		task_ptr->timedout = FALSE;
		task_ptr = task_ptr->next;
	}
}

static void timeoutTasks() {
	task_eptr = task_head;
	while (task_eptr != NULL) {
		if (task_eptr->active == TRUE && task_eptr->timeout == timer_ticks) {
			task_eptr->timedout = TRUE;
			task_eptr->timeout = task_eptr->ticks + timer_ticks;
		}
		task_eptr = task_eptr->next;
	}
}

// 100us timer increments timer_ticks
// Timer interrupt service routine
#pragma vector=SCHEDULER_VECTOR
__interrupt void SCHEDULER_ISR (void)
{
	timer_ticks++;
	timeoutTasks();
}
