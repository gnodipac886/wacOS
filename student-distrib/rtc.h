#ifndef _RTC_H
#define _RTC_H


/* rtc.h - Defines RTC structures and functions. */
#define RTC_IO_PORT     0x70
#define CMOS_IO_PORT    0x71
#define RTC_STATUS_REG  0x80
#define RTC_IRQ         0x08
/* rtc_init: Initializes RTC */
void __rtc_init__();
void handle_rtc_interrupt();
#endif /* _RTC_H */
