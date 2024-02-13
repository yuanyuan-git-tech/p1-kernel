#ifndef	_TIMER_H
#define	_TIMER_H

/* These are for "System Timer". See timer.c for details */
void timer_init ( void );
void handle_timer_irq ( void );

/* below are for Arm generic timers */
void generic_timer_init ( void );
void handle_generic_timer_irq ( void );
void set_timer_interval( unsigned long );
void clear_timer_interrupt( void );

extern void gen_timer_init();
extern void gen_timer_reset();

#endif  /*_TIMER_H */
