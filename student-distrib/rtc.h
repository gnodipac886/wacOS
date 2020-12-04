#ifndef _RTC_H
#define _RTC_H


/* rtc.h - Defines RTC structures and functions. */
#define RTC_IO_PORT         0x70        // RTC Port
#define CMOS_IO_PORT        0x71        // CMOS Port
#define RTC_STATUS_REG      0x80        // RTC register
#define RTC_IRQ             0x08        // RTC IRQ number
#define RTC_RATE_MAX        0x10        // Max rate for frequency is 16.
#define DEVICE_MAX_FREQ     1024        // RTC Device Max Frequency = 1024 Hz (Device is always set to this)
#define VIRT_DEFAULT_FREQ   2           // All programs are by default set to 2Hz (Virtualized RTC)

/* rtc_init: Initializes RTC */
void __init_rtc__();
/* handle_rtc_interrupt: processes rtc interrupt.*/
void handle_rtc_interrupt();
/* rtc_open: Opens rtc file */
int _rtc_open();
/* rtc_close: Closes rtc file */
int _rtc_close();
/* rtc_read: returns after an interrupt occurs */
int _rtc_read();
/* rtc_write: sets rtc frequency*/
int _rtc_write(void* buf);
#endif /* _RTC_H */
