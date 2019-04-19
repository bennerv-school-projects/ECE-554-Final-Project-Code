#include "interrupt.h"
#include <Arduino.h>

// Contains all the information on the three priorities and tasks associated with them
task_info * timerCurrentTasks[3];

/**
 * Initializes the high priority timer and sets it to trigger an interrupt on overflow
 */
void initialize_timer3_interrupt() {
  TCCR3A = 0; // clean registers
  TCCR3B = 0;
  TCNT3 = 64911; // preset value for overflow

  TCCR3B |= (1 << CS12); // 256 prescaler
  TIMSK3 |= (1 << TOIE3); // enable timer overflow interrupt
}


/**
 * The high priority ISR.  
 * Utilizes an overflow interrupt scheme and runs at a fixed interval of 100Hz
 * 
 * Non-preemptive as it is the highest priority
 */
ISR(TIMER3_OVF_vect) {
  if (timerCurrentTasks[HIGH_PRIORITY] == NULL) {
    return;
  }

  // Reset the timer value for overflow
  TCNT3 = 64911;

  // Call the task and point it to the next one
  timerCurrentTasks[HIGH_PRIORITY]->function();
  timerCurrentTasks[HIGH_PRIORITY] = timerCurrentTasks[HIGH_PRIORITY]->next;
}

/**
 * Initializes the medium priority timer and sets it to trigger an interrupt on overflow
 */
void initialize_timer4_interrupt() {
 
  TCCR4A = 0;
  TCCR4B = 0;
  TCNT4 = 64286; // preset value for overflow

  TCCR4B |= (1 << CS12); // 256 prescaler
  TIMSK4 |= (1 << TOIE4);
}

/**
 * The Medium priority ISR.  
 * Utilizes an overflow interrupt scheme and runs at a fixed interval of 50Hz
 * 
 */
ISR(TIMER4_OVF_vect, ISR_NOBLOCK) {
  if (timerCurrentTasks[MEDIUM_PRIORITY] == NULL) {
    return;
  }

  // Reset the timer value for overflow
  TCNT4 = 64286;

  // Call the task and point it to the next one
  timerCurrentTasks[MEDIUM_PRIORITY]->function();
  timerCurrentTasks[MEDIUM_PRIORITY] = timerCurrentTasks[MEDIUM_PRIORITY]->next;
}

/**
 * Initializes the low priority timer and sets it to trigger an interrupt on overflow
 */
void initialize_timer5_interrupt() {
  TCCR5A = 0; // clean registers
  TCCR5B = 0; // clean timer registers
  TCNT5 = 59286; // overflow preset number

  TCCR5B |= (1 << CS12); // 256 prescaler
  TIMSK5 |= (1 << TOIE5); // enable timer overflow interrupt
}

/**
 * The Medium priority ISR.  
 * Utilizes an overflow interrupt scheme and runs at a fixed interval of 10Hz
 * 
 */
ISR(TIMER5_OVF_vect, ISR_NOBLOCK) {

  // Check if we have any tasks available to run, or if the interrupt
  // flag is set for the medium priority task to make the low priority task preemptive
  if (timerCurrentTasks[LOW_PRIORITY] == NULL || TOV4) {
    return;
  }

  TCNT5 = 59286; // overflow preset number

  // Call the task and point it to the next one
  timerCurrentTasks[LOW_PRIORITY]->function();
  timerCurrentTasks[LOW_PRIORITY] = timerCurrentTasks[LOW_PRIORITY]->next;
}

/**
 * Allows for creation of a task with specifying a priority.  Based on that priority, it will
 * be associated with a timer that runs on a specific interval.    
 * 
 * Params: 
 *   - functionPointer - a function pointer to the task which should be run
 *   - priority - the priority of the task to be run
 */
void createTask(void ( * functionPointer )( void ), priority taskPriority) {
  if (timerCurrentTasks[taskPriority] == NULL) {
    timerCurrentTasks[taskPriority] = (task_info*) malloc(sizeof(task_info)); // dynamically allocate
    timerCurrentTasks[taskPriority]->function = functionPointer; // set the function pointer on task to call later
    timerCurrentTasks[taskPriority]->function();
    timerCurrentTasks[taskPriority]->next = timerCurrentTasks[taskPriority]; // make it circular (reference head aka itself)
  } else {
    task_info * tempHead = timerCurrentTasks[taskPriority]->next; // timerCurrentTasks->next is always going to reference head in this context
    timerCurrentTasks[taskPriority]->next = (task_info*) malloc(sizeof(task_info)); // dynamically allocate a new task
    timerCurrentTasks[taskPriority] = timerCurrentTasks[taskPriority]->next; // Update reference of current task
    timerCurrentTasks[taskPriority]->function = functionPointer; // set the function pointer on task to call later
    timerCurrentTasks[taskPriority]->function();
    timerCurrentTasks[taskPriority]->next = tempHead; // make it circular
  }
}
