/* rtc.h - Defines RTC structures and functions. */
#define RTC_IO_PORT     0x70
#define CMOS_IO_PORT    0x71
#define RTC_STATUS_REG  0x80
/* rtc_init: Initializes RTC */
void rtc_init();
