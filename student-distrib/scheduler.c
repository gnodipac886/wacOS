#include "scheduler.h"
#include "system_calls.h"
#include "lib.h"
#include "pit.h"
#include "keyboard.h"
#include "paging.h"
#include "x86_desc.h"
#include "i8259.h"

/* size of pcb_arr = MAX_TERMINALS = 6*/
// pcb_t** pcb_arr;
extern pcb_t* pcb_arr[MAX_TASKS];
int pcb_process[MAX_TERMINALS] = {FAILURE, FAILURE, FAILURE};
int pid_tracker[MAX_TERMINALS];											// keep track of which pid is currently on the terminals
int setup_counter;
int curr_scheduled;
int base_shell_flag;

void __init_scheduler__(){
	int i;				// loop counter
	setup_counter = 0;
	curr_scheduled = 0;
	base_shell_flag = 0;
	// pcb_arr = _get_pcb_arr();
	// init to pid i for all terminals
	for(i = 0; i < MAX_TERMINALS; i++){
		pid_tracker[i] = i;
	}
}

void switch_process(int curr_pid, int next_pid){
	/*execute shell on all terminals in first epoch*/
	base_shell_flag = 0;
	pcb_t* curr_pcb = pcb_arr[curr_pid];
	pcb_t* next_pcb = pcb_arr[next_pid];

	if(setup_counter < MAX_TERMINALS){
		pcb_arr[setup_counter] = (pcb_t*)(KER_BOTTOM - (setup_counter + 1) * KER_STACK_SIZE);
		curr_pcb = pcb_arr[setup_counter];
		// store current kernel stack info - esp and ebp
		asm volatile(
			"movl	%%esp, 	%0;"
			"movl	%%ebp, 	%1;"
			"movl 	%%eip, 	%2;"
			:"=g"(curr_pcb->curr_esp), "=g"(curr_pcb->curr_ebp), "=g"(curr_pcb->curr_eip)// outputs - temp vars to be used to set pcb values
		);
		text_screen_map_update(curr_scheduled, get_curr_screen());
		curr_scheduled = (curr_scheduled + 1) % 3;
		pcb_process[setup_counter] = setup_counter;
		setup_counter++;
		base_shell_flag = 1;
		send_eoi(PIT_IRQ);					        									// Enable interrupt for PIT on PIC
		sti();
		execute((uint8_t*)"shell");
		return;
	} 

	curr_scheduled = (curr_scheduled + 1) % 3;

	/*update text-screen 4kB paging*/
	text_screen_map_update(curr_scheduled, get_curr_screen());

	if(curr_pcb->vidmap_page_flag){														// update the vid map if the program is using it
		vidmap_update();
	}


	// store current kernel stack info - esp and ebp
	// load the next process's esp and ebp
	asm volatile(
		"movl	%%esp, 	%0;"
		"movl	%%ebp, 	%1;"
		"movl	%%eip, 	%2;"
		"movl 	%3,		%%esp;"
		"movl	%4,		%%ebp;"
		"movl	%5,		%%eip;"
		:"=g" (curr_pcb->curr_esp), "=g" (curr_pcb->curr_ebp), "=g"(curr_pcb->curr_eip)	// outputs - temp vars to be used to set pcb values
		:"r" (next_pcb->curr_esp), "r" (next_pcb->curr_ebp), "r"(next_pcb->curr_eip)
		:"%esp", "%ebp", "%eip"
	);

	// restore next process's tss
	tss.esp0 = KER_BOTTOM - next_pid * KER_STACK_SIZE - sizeof(unsigned long);
	tss.ss0 = KERNEL_DS;

	// flush the TLB
	flush_tlb();
}

/* _get_pcb_process
 *      Inputs: none
 *      Return Value: pcb_process
 *      Function: helper function to retrieve pcb_process
 *      Side Effects: none
 */
int* _get_pcb_process(){
	return pcb_process;
}

/* is_active_process
 *      Inputs: screen_num
 *      Return Value: is active: whether shell is running on that screen
 *      Function: helper function to retrieve pcb_process
 *      Side Effects: none
 */
int is_active_process(int screen_num){
    return pcb_process[screen_num];
}

/* get_curr_scheduled
 *      Inputs: none
 *      Return Value: curr_screen - the current screen we are on
 *      Function:
 *      Side Effects: none
 */
int get_curr_scheduled(){
	return curr_scheduled;
}

/* _get_base_shell_flag
 *      Inputs: none
 *      Return Value: pointer to base_shell_flag
 *      Function: helper function to retrieve the base_shell_flag
 *      Side Effects: none
 */
int* _get_base_shell_flag(){
	return &(base_shell_flag);
}

/* _get_pid_tracker
 *      Inputs: none
 *      Return Value: pointer to pid_tracker
 *      Function: helper function to retrieve the pid_tracker
 *      Side Effects: none
 */
int* _get_pid_tracker(){
	return pid_tracker;
}
