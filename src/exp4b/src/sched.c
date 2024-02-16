#include "sched.h"
#include "irq.h"
#include "printf.h"
#include "timer.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct * task[NR_TASKS] = {&(init_task), };
int nr_tasks = 1;
struct context_switch_trace trace_buffer[TRACE_SIZE];
int trace_index = 0;

void record_context_switch(int out_id, unsigned long out_pc, unsigned long out_sp,
                           int in_id, unsigned long in_pc, unsigned long in_sp) {
    if (trace_index < TRACE_SIZE) {
        // Directly access the structure in the pre-allocated array
        trace_buffer[trace_index].timestamp = get_time_ms();
        trace_buffer[trace_index].task_out_id = out_id;
        trace_buffer[trace_index].task_out_pc = out_pc;
        trace_buffer[trace_index].task_out_sp = out_sp;
        trace_buffer[trace_index].task_in_id = in_id;
        trace_buffer[trace_index].task_in_pc = in_pc;
        trace_buffer[trace_index].task_in_sp = in_sp;
        // Increment the index for the next use
        trace_index++;
    }
}


void preempt_disable(void)
{
	current->preempt_count++;
}

void preempt_enable(void)
{
	current->preempt_count--;
}


void _schedule(void)
{
	/* ensure no context happens in the following code region
		we still leave irq on, because irq handler may set a task to be TASK_RUNNING, which 
		will be picked up by the scheduler below */

	preempt_disable(); 
	int next,c;
	struct task_struct * p;
	while (1) {
		c = -1; // the maximum counter of all tasks 
		next = 0;

		/* Iterates over all tasks and tries to find a task in 
		TASK_RUNNING state with the maximum counter. If such 
		a task is found, we immediately break from the while loop 
		and switch to this task. */

		for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if (p && p->state == TASK_RUNNING && p->counter > c) {
				c = p->counter;
				next = i;
			}
		}
		if (c) {
			break;
		}

		/* If no such task is found, this is either because i) no 
		task is in TASK_RUNNING state or ii) all such tasks have 0 counters.
		in our current implemenation which misses TASK_WAIT, only condition ii) is possible. 
		Hence, we recharge counters. Bump counters for all tasks once. */
		
		for (int i = 0; i < NR_TASKS; i++) {
			p = task[i];
			if (p) {
				p->counter = (p->counter >> 1) + p->priority;
			}
		}
	}

	/* 
		After 50 context switches, print out a list of switch records to UART
	*/
	if (trace_index == TRACE_SIZE - 1) {
		disable_irq();
		print_trace_records();
		while (1) {
		}
		enable_irq();
	}

	switch_to(task[next]);
	preempt_enable();
}

void schedule(void)
{
	current->counter = 0;
	_schedule();
}

void switch_to(struct task_struct * next) 
{
	if (current == next) 
		return;
	
	int out_id = getpid();
    int in_id = -1;
    for (int i = 0; i < NR_TASKS; i++) {
        if (task[i] == next) {
            in_id = i;
            break;
        }   
    }

	unsigned long out_pc = current->cpu_context.pc;
    unsigned long out_sp = current->cpu_context.sp;
    unsigned long in_pc = get_task_pc(next);
    unsigned long in_sp = get_task_sp(next);

	record_context_switch(out_id, out_pc, out_sp, in_id, in_pc, in_sp);

	struct task_struct * prev = current;
	current = next;

	/*	 
		below is where context switch happens. 

		after cpu_switch_to(), the @prev's cpu_context.pc points to the instruction right after  
		cpu_switch_to(). this is where the @prev task will resume in the future. 
		for example, shown as the arrow below: 

			cpu_switch_to(prev, next);
			80d50:       f9400fe1        ldr     x1, [sp, #24]
			80d54:       f94017e0        ldr     x0, [sp, #40]
			80d58:       9400083b        bl      82e44 <cpu_switch_to>
		==> 80d5c:       14000002        b       80d64 <switch_to+0x58>
	*/
	cpu_switch_to(prev, next);  /* will branch to @next->cpu_context.pc ...*/
}

void schedule_tail(void) {
	preempt_enable();
}


void timer_tick()
{
	--current->counter;
	if (current->counter > 0 || current->preempt_count > 0) 
		return;
	current->counter=0;

	/* Note: we just came from an interrupt handler and CPU just automatically disabled all interrupts. 
		Now call scheduler with interrupts enabled */
	enable_irq();
	_schedule();
	/* disable irq until kernel_exit, in which eret will resort the interrupt flag from spsr, which sets it on. */
	disable_irq(); 
}


int getpid(void) {
	for (int i = 0; i < NR_TASKS; i++) {
			if (task[i] == current) {
				return i;
			}
	}
	return -1;
}

void print_trace_records(void) {
    for (int i = 0; i < trace_index; i++) {
		printf("\n%d from task%d (PC 0x%x SP 0x%x) to task%d (PC 0x%x SP 0x%x)",
            trace_buffer[i].timestamp,
            trace_buffer[i].task_out_id, trace_buffer[i].task_out_pc, trace_buffer[i].task_out_sp,
            trace_buffer[i].task_in_id, trace_buffer[i].task_in_pc, trace_buffer[i].task_in_sp);
        }
}
