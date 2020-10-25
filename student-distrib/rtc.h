#ifndef _RTC_H
#define _RTC_H


/* rtc.h - Defines RTC structures and functions. */
#define RTC_IO_PORT     0x70
#define CMOS_IO_PORT    0x71
#define RTC_STATUS_REG  0x80
#define RTC_IRQ         0x08
#define RTC_RATE_MAX    0x10    // Max rate for frequency is 16.

/* rtc_init: Initializes RTC */
void __rtc_init__();
/* handle_rtc_interrupt: processes rtc interrupt.*/
void handle_rtc_interrupt();
/* rtc_open: Opens rtc file */
void rtc_open();
/* rtc_close: Closes rtc file */
void rtc_close();
/* rtc_read: returns after an interrupt occurs */
int rtc_read();
/* rtc_write: sets rtc frequency*/
int rtc_write(int freq);
#endif /* _RTC_H */
