/*
 * scheduler.h
 *
 *  Created on: Dec 19, 2013
 *      Author: a0273922
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SCHEDULER_VECTOR 	TIMER0_A0_VECTOR
#define SCHEDULER_CCR0		TA0CCR0
#define SCHEDULER_EX0		TA0EX0
#define SCHEDULER_CTL		TA0CTL
#define SCHEDULER_CCTL0		TA0CCTL0

// divisor from SMCLK
#define SCHEDULER_DIV       (240)

typedef struct scheduled_task {
	uint8_t pid;
	uint8_t active;
	uint8_t timedout;
	uint32_t ticks;
	uint32_t timeout;
	void (* function) (struct scheduled_task * self);
	struct scheduled_task * next;
	struct scheduled_task * prev;
} scheduled_task_t;

void initScheduler();

void addTask(scheduled_task_t * task);
void removeTask(scheduled_task_t * task);
void removeAllTasks();

void executeTasks();

#endif /* SCHEDULER_H_ */
