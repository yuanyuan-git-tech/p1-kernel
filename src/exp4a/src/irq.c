#include "utils.h"
#include "printf.h"
#include "timer.h"
#include "entry.h"
#include "peripherals/irq.h"
#include "sched.h"

const char *entry_error_messages[] = {
    "SYNC_INVALID_EL1t",
    "IRQ_INVALID_EL1t",		
    "FIQ_INVALID_EL1t",		
    "ERROR_INVALID_EL1T",		

    "SYNC_INVALID_EL1h",		
    "IRQ_INVALID_EL1h",		
    "FIQ_INVALID_EL1h",		
    "ERROR_INVALID_EL1h",		

    "SYNC_INVALID_EL0_64",		
    "IRQ_INVALID_EL0_64",		
    "FIQ_INVALID_EL0_64",		
    "ERROR_INVALID_EL0_64",	

    "SYNC_INVALID_EL0_32",		
    "IRQ_INVALID_EL0_32",		
    "FIQ_INVALID_EL0_32",		
    "ERROR_INVALID_EL0_32"	
};

void enable_interrupt_controller()
{
   // Enables Core 0 Timers interrupt control for the generic timer
   put32(TIMER_INT_CTRL_0, TIMER_INT_CTRL_0_VALUE);
}

void handle_irq(void)
{
    // Each Core has its own pending local intrrupts register
    unsigned int irq = get32(INT_SOURCE_0);
    unsigned int interval = (1 << 26);
    switch (irq) {
        case (GENERIC_TIMER_INTERRUPT):
            gen_timer_reset(interval);
            check_wait_to_ready_tasks();
            break;
        default:
            printf("Unknown pending irq: %x\r\n", irq);
    }
}
void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
    printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

