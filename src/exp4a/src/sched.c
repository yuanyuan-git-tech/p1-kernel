#include "sched.h"
#include "irq.h"
#include "printf.h"
#include "timer.h"
#include <limits.h>

static struct task_struct init_task = INIT_TASK; // the very first task with its task_struct values
struct task_struct *current = &(init_task);		 // points to the currently running task. when kernel boots, sets to init_task
struct task_struct *task[NR_TASKS] = {
	&(init_task),
}; // holds all task_strcuts. only has init_ask at beginning
int nr_tasks = 1;

void _schedule(void)
{
	int next, c;
	struct task_struct *p;
	while (1)
	{
		c = -1; // the maximum counter found so far
		next = 0;

		/* Iterates over all tasks and tries to find a task in
		TASK_RUNNING state with the maximum counter. If such
		a task is found, we immediately break from the while loop
		and switch to this task. */

		for (int i = 0; i < NR_TASKS; i++)
		{
			p = task[i];
			if (p && p->state == TASK_RUNNING && p->counter > c)
			{ /* NB: p->counter always be non negative */
				c = p->counter;
				next = i;
			}
		}
		if (c)
		{ /* found a RUNNING/READY task w/ the most positive counter.  NB: c won't be -1 as counter always nonnegative */
			break;
		}

		/* If no such task is found, this is either because i) no
		task is in TASK_RUNNING state or ii) all such tasks have 0 counters.
		in our current implemenation which misses TASK_WAIT, only condition ii) is possible.
		Hence, we recharge counters. Bump counters for all tasks once. */
		for (int i = 0; i < NR_TASKS; i++)
		{
			p = task[i];
			if (p && p->state == TASK_WAIT) {
				continue;
			} else if (p) {
				p->counter = (p->counter >> 1) + p->priority; // The increment depends on a task's priority.
			}
		}

		/* loops back to pick the next task */
	}
	switch_to(task[next]);
}

void schedule(void)
{
	current->counter = 0;
	_schedule();
}

// where the multitasking magic happens
void switch_to(struct task_struct *next)
{
	if (current == next)
		return;
	struct task_struct *prev = current;
	current = next;
	cpu_switch_to(prev, next); // do context switch
}

void schedule_tail(void)
{
	/* nothing */
}

void update_timer_for_sleep() {
    unsigned long nearest_wake_up = ULONG_MAX;
	struct task_struct *p;	
    for (int i = 0; i < NR_TASKS; i++) {
		p = task[i];
        if (p && p->state == TASK_WAIT && p->wake_up_times < nearest_wake_up) {
            nearest_wake_up = p->wake_up_times;
        }
    }
    if (nearest_wake_up != ULONG_MAX) {
        set_timer_interval(nearest_wake_up); // Set the hardware timer
    }
}

void sleep(unsigned int sleep_time_sec)
{
	unsigned long timer_freq = get_timer_frequency();
	unsigned long interval = sleep_time_sec * timer_freq;
	unsigned long current_time = get_system_time_ticks();
    unsigned long wake_up_time = current_time + interval;
	current->state = TASK_WAIT;
	current->wake_up_times = wake_up_time;
	printf("\nThe task will wake up at %u ticks\r\n", current->wake_up_times);
	update_timer_for_sleep();
}

void check_wait_to_ready_tasks( void ) {
	unsigned long current_time = get_system_time_ticks();
    unsigned long next_wake_up = ULONG_MAX;
    struct task_struct *p;
	printf("Timer IRQ received...\n");
	clear_timer_interrupt();
    for (int i = 0; i < NR_TASKS; i++) {
		p = task[i];
		// Wake up tasks whose wake-up time has passed
        if (p && p->state == TASK_WAIT && p->wake_up_times <= current_time) {
            p->state = TASK_RUNNING;
        }
		// Find the next task to wake up
        else if (p && p->state == TASK_WAIT && p->wake_up_times < next_wake_up) {
            next_wake_up = p->wake_up_times;
        }
    }
	if (next_wake_up != ULONG_MAX) {
        unsigned long interval = next_wake_up > current_time ? next_wake_up - current_time : 0;
        set_timer_interval(interval); // Set timer for the next wake-up
    }
}

int no_runnable_tasks(void) {
	struct task_struct *p;
	for (int i = 1; i < NR_TASKS; i++) {
		p = task[i];
		if (p && p->state == TASK_RUNNING) { 
			return 0;
		}
	}
	return 1;
}
