#ifndef	_TIMER_H
#define	_TIMER_H

void timer_init ( void );
void handle_timer_irq ( void );

void generic_timer_init ( void );
void handle_generic_timer_irq ( void );

extern void gen_timer_init();
extern void gen_timer_reset(int interval);

extern unsigned long get_time_ms();
extern unsigned long get_system_timer_ticks();
extern unsigned long get_system_timer_frq();

#endif  /*_TIMER_H */
