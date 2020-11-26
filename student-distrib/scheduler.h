#ifndef _SCHDULER_H
#define _SCHDULER_H

#define MAX_TERMINALS    3          // Max terminals supported

/* scheduler_init: Initializes scheduler*/
void __init_scheduler__();
/* switch_process: Switches to the next process on the queue*/
void switch_process(int curr_pid, int next_pid);
/* is_active_process: Checks if process is currently active and running*/
int is_active_process(int screen_num);
/* helper function to retrieve pcb_process */
int* _get_pcb_process();
int get_curr_scheduled();
int* _get_base_shell_flag();
int* _get_pid_tracker();
#endif /* _SCHDULER_H */
