/* rtc.h - Defines RTC structures and functions. */

/* This structure is used to return RTC's time */
typedef struct rtc_time {
	int time_sec;
    int time_min;
    int time_hour;
    int time_mday;
    int time_mon;
    int time_year;
};
