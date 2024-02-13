#include "utils.h"
#include "printf.h"
#include "sched.h"
#include "peripherals/timer.h"
#include "timer.h"

#ifdef USE_QEMU
int interval = (1 << 26); // xzl: around 1 sec
#else
int interval = 1 * 1000 * 1000; // xzl: around 1 sec
#endif

/* 	These are for Arm generic timers. 
	They are fully functional on both QEMU and Rpi3.
	Recommended.
*/
void generic_timer_init ( void )
{
	gen_timer_init();
}

void handle_generic_timer_irq( void ) 
{
	// TODO: In order to implement sleep(t), you should calculate @interval based on t, 
	// instead of having a fixed @interval which triggers periodic interrupts
	check_wait_to_ready_tasks();
}

void set_timer_interval( unsigned long timer_interval ) {
	gen_timer_reset(timer_interval);
}

void clear_timer_interrupt() {
    unsigned long cntp_ctl_value;
    asm volatile("mrs %0, CNTP_CTL_EL0" : "=r" (cntp_ctl_value));
    cntp_ctl_value &= ~2; // Clear the ISTATUS bit
    asm volatile("msr CNTP_CTL_EL0, %0" :: "r"(cntp_ctl_value));
}
