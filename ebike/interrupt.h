#ifndef __EBIKE_INTERRUPTS__
#define __EBIKE_INTERRUPTS__

typedef enum {HIGH_PRIORITY, MEDIUM_PRIORITY, LOW_PRIORITY} priority;

void initialize_timer3_interrupt();
void initialize_timer4_interrupt();
void initialize_timer5_interrupt();

typedef struct task_info {
  task_info * next;
  void (*function)();
} task_info;

void createTask(void ( * functionPointer )( void ), priority taskPriority);

#endif
