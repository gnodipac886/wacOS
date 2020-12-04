#ifndef _PIT_H
#define _PIT_H

/* pit.h - Defines PIT structures and functions. */
#define PIT_FREQ            0x64            // 100(0x64) Hz
#define PIT_MS              0xA             // 10(0xA) ms between each signal  
#define PIT_IRQ             0x00            // IRQ0 on PIC
#define PIT_CMD_REG         0x43            // Command register at 0x43
#define PIT_DATA_REG_0      0x40            // Channel 0's Data register is at 0x40
#define PIT_DEF_FREQ        0x1234DC        // Default Freq Value 
#define PIT_CMD_BYTE        0x36            // Command byte for Counter 0, BCD = 0, Read/Write mode = 3, Square wave mode
 
/* pit_init: Initializes PIT */
void __init_pit__();
/* handle_pit_interrupt: processes pit interrupt.*/
void handle_pit_interrupt();

#endif /* _PIT_H */
