// rtc.h
#ifndef RTC_H
#define RTC_H

#include <stdint.h>

// Date and time structure
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} datetime_t;

// RTC functions
void rtc_get_time(datetime_t *time);        // Get current date and time
uint8_t cmos_read(uint8_t address);         // Read from CMOS register
char* get_day_name(uint8_t day);            // Get day name from day number
char* get_month_name(uint8_t month);        // Get month name from month number

#endif