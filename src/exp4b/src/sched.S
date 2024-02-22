#include "sched.h"

.globl cpu_switch_to
cpu_switch_to:
	mov	x10, #THREAD_CPU_CONTEXT
	add	x8, x0, x10
	mov	x9, sp
	stp	x19, x20, [x8], #16		// store callee-saved registers
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	x30, [x8]				// x30=LR. this saves in task_struct the "return-to" address, i.e. the instruction right after the cpu_switch_to() callsite 
	add	x8, x1, x10
	ldp	x19, x20, [x8], #16		// restore callee-saved registers
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x9, [x8], #16
	ldr	x30, [x8]				// load LR from memory ...
	mov	sp, x9
	ret							// branch to LR (which was just loaded from memory)

// Registers x19 to x30 are callee-saved registers.
// x30 (the link register) holds the return address for the function call, saving it ensures the task can resume execution from the correct point.
// The stack pointer for the next task (x1's task) is loaded into sp.

// The x0 register is expected to hold the pointer to the current task's structure, while x1 will hold the pointer to the next task's structure.
// add x8, x0, x10: This instruction calculates the address where the CPU context needs to be saved within the current task's structure. It adds the offset stored in x10 to the base address of the current task's structure (presumably stored in x0) and stores the result in x8. x8 now points to the location where the registers should be saved.

// stp x19, x20, [x8], #16: This instruction stores the pair of registers x19 and x20 at the location pointed to by x8, and then automatically increments x8 by 16 bytes (the size of two registers). The stp instruction is used for "store pair", which is efficient for saving two registers at once.
// The instruction str x30, [x8] is used to store the content of register x30 into the memory location pointed to by x8.