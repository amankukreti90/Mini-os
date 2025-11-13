// rtc.c
#include "rtc.h"
#include "../io.h"

// Read from CMOS register
uint8_t cmos_read(uint8_t address) {
    outb(0x70, address);
    return inb(0x71);
}

// Wait for RTC to not be updating
static void rtc_wait_update(void) {
    while (cmos_read(0x0A) & 0x80);
}

// Convert BCD to binary
static uint8_t bcd_to_bin(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

// Get current time from RTC
void rtc_get_time(datetime_t *time) {
    // Wait for RTC to not be updating
    rtc_wait_update();
    
    // Read RTC registers multiple times until we get consistent values
    uint8_t last_second, last_minute, last_hour;
    uint8_t second, minute, hour;
    
    do {
        last_second = cmos_read(0x00);
        last_minute = cmos_read(0x02);
        last_hour = cmos_read(0x04);
        
        // Read again to check consistency
        second = cmos_read(0x00);
        minute = cmos_read(0x02);
        hour = cmos_read(0x04);
    } while (last_second != second || last_minute != minute || last_hour != hour);
    
    time->second = second;
    time->minute = minute;
    time->hour = hour;
    time->day = cmos_read(0x07);
    time->month = cmos_read(0x08);
    time->year = cmos_read(0x09);

    uint8_t status_b = cmos_read(0x0B);
    
    // Convert from BCD to binary if needed
    if (!(status_b & 0x04)) {
        time->second = bcd_to_bin(time->second);
        time->minute = bcd_to_bin(time->minute);
        time->hour = bcd_to_bin(time->hour);
        time->day = bcd_to_bin(time->day);
        time->month = bcd_to_bin(time->month);
        time->year = bcd_to_bin(time->year);
    }

    // Check if 12-hour format and convert to 24-hour
    if (status_b & 0x02) {
        if (time->hour & 0x80) {
            time->hour &= 0x7F;
            
            if (!(status_b & 0x04)) {
                time->hour = bcd_to_bin(time->hour);
            }
            
            if (time->hour != 12) {
                time->hour += 12;
            }
        } else {
            if (time->hour == 12) {
                time->hour = 0;
            }
        }
    }

    // Timezone correction: UTC to local time (IST = UTC+5:30)
    time->hour += 5;
    time->minute += 30;
    
    // Handle minute overflow
    if (time->minute >= 60) {
        time->minute -= 60;
        time->hour += 1;
    }
    
    // Handle hour overflow
    if (time->hour >= 24) {
        time->hour -= 24;
        time->day += 1;
    }
    
    // Convert 2-digit year to 4-digit
    time->year += 2000;
}

// Get day name from day number
char* get_day_name(uint8_t day) {
    static char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return days[day % 7];
}

// Get month name from month number
char* get_month_name(uint8_t month) {
    static char* months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    return (month >= 1 && month <= 12) ? months[month - 1] : "???";
}