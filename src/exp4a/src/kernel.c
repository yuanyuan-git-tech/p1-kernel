#include "printf.h"
#include "utils.h"
#include "timer.h"
#include "irq.h"
#include "fork.h"
#include "sched.h"
#include "mini_uart.h"

#ifdef USE_LFB
#include "lfb.h"
#endif

#ifdef USE_QEMU
#define CHAR_DELAY (5 * 5000000)
#else
#define CHAR_DELAY (1000000)
#endif

struct process_args {
    char *array;
	int sleep_time_sec;
};

void process(struct process_args *args) 
{
#ifdef USE_LFB // (optional) determine the init locations on the graphical console
	int scr_x, scr_y; 
	char c; 
	if (array[0] == '1') {
		scr_x = 0; scr_y = 320; 
	} else {
		scr_x = 0; scr_y = 480; 
	}
#endif 
	char *array = args->array;
	int sleep_time_sec = args->sleep_time_sec;
	
	while (1){
		for (int i = 0; i < 5; i++){
			uart_send(array[i]);
#ifdef USE_LFB  // (optional) output to the graphical console
			c = array[i+1]; array[i+1]='\0';
			lfb_print_update(&scr_x, &scr_y, array+i);
			array[i+1] = c; 
			if (scr_x > 1024)
				lfb_print_update(&scr_x, &scr_y, "\n");
#endif
			delay(CHAR_DELAY);
		} 
		sleep(sleep_time_sec);
		schedule(); // yield
	}

	// For now, all the tasks run in an infinite loop and never returns. 
	// We will handle task termination in future experiments
}

void kernel_main(void)
{
	uart_init();
	init_printf(0, putc);

	printf("kernel boots\r\n");	

	// Below, irq is off by default, b/c it is not needed for cooperative scheduling as
	// in the proj description. but to implement things like sleep() it will be needed. 
	// if so, enable irq and take care of irq handling
	irq_vector_init();
	generic_timer_init();
	enable_interrupt_controller();
	enable_irq();		

#ifdef USE_LFB // (optional) init output to the graphical console
	lfb_init(); 
	lfb_showpicture();
	lfb_print(0, 240, "kernel boots");
#endif		

	static struct process_args arg_1 = {"12345", 6};
	static struct process_args arg_2 = {"abcde", 2};
	
	int res = copy_process((unsigned long)&process, (unsigned long)&arg_1);
	if (res != 0) {
		printf("error while starting process 1");
		return;
	}
	
	res = copy_process((unsigned long)&process, (unsigned long)&arg_2);
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}

	while (1){
		schedule();
	}	
}
