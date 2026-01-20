#include "validation.h"
#include <ctype.h>
#include <regex.h>

bool validate_email(const char *email) {
    if (!email || strlen(email) == 0) {
        return false;
    }
    
    // Basic email validation: must contain @ and . after @
    const char *at = strchr(email, '@');
    if (!at || at == email) {
        return false;
    }
    
    const char *dot = strchr(at, '.');
    if (!dot || dot == at + 1 || *(dot + 1) == '\0') {
        return false;
    }
    
    return true;
}

bool validate_password(const char *password, int min_length) {
    if (!password) {
        return false;
    }
    
    size_t len = strlen(password);
    if (len < (size_t)min_length) {
        return false;
    }
    
    // For passwords >= 8 characters, require at least one digit or special char
    if (min_length >= 8) {
        bool has_digit_or_special = false;
        for (size_t i = 0; i < len; i++) {
            if (isdigit(password[i]) || !isalnum(password[i])) {
                has_digit_or_special = true;
                break;
            }
        }
        if (!has_digit_or_special) {
            return false;
        }
    }
    
    return true;
}

bool validate_required(const char *value) {
    if (!value) {
        return false;
    }
    
    // Trim leading spaces
    while (*value && isspace(*value)) {
        value++;
    }
    
    return strlen(value) > 0;
}

bool validate_date(const char *date_str) {
    if (!date_str || strlen(date_str) == 0) {
        return false;
    }
    
    int year, month, day;
    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }
    
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    
    // Check days in month
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Check for leap year
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        days_in_month[1] = 29;
    }
    
    if (day > days_in_month[month - 1]) {
        return false;
    }
    
    return true;
}

bool validate_time(const char *time_str) {
    if (!time_str || strlen(time_str) == 0) {
        return false;
    }
    
    int hour, minute;
    if (sscanf(time_str, "%d:%d", &hour, &minute) != 2) {
        return false;
    }
    
    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;
    
    return true;
}

bool validate_datetime(const char *datetime_str) {
    if (!datetime_str || strlen(datetime_str) == 0) {
        return false;
    }
    
    char date_part[50] = {0};
    char time_part[50] = {0};
    
    if (sscanf(datetime_str, "%s %s", date_part, time_part) != 2) {
        return false;
    }
    
    return validate_date(date_part) && validate_time(time_part);
}

bool validate_roll_number(const char *roll_number) {
    return validate_required(roll_number);
}

bool validate_phone(const char *phone) {
    if (!validate_required(phone)) {
        return false;
    }
    
    // Check if contains only digits, spaces, +, -, (, )
    for (const char *p = phone; *p; p++) {
        if (!isdigit(*p) && *p != ' ' && *p != '+' && *p != '-' && *p != '(' && *p != ')') {
            return false;
        }
    }
    
    return true;
}

bool parse_datetime(const char *datetime_str, time_t *result) {
    if (!datetime_str || !result) {
        return false;
    }
    
    struct tm tm = {0};
    int year, month, day, hour = 0, minute = 0, second = 0;
    
    // Try full datetime format first
    int parsed = sscanf(datetime_str, "%d-%d-%d %d:%d:%d", 
                       &year, &month, &day, &hour, &minute, &second);
    
    if (parsed < 3) {
        // Try date only format
        if (sscanf(datetime_str, "%d-%d-%d", &year, &month, &day) != 3) {
            return false;
        }
    }
    
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = -1;
    
    *result = mktime(&tm);
    return (*result != -1);
}

void format_datetime(time_t timestamp, char *output, size_t size) {
    if (!output || size == 0) {
        return;
    }
    
    struct tm *tm_info = localtime(&timestamp);
    if (tm_info) {
        strftime(output, size, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        output[0] = '\0';
    }
}

void format_date(time_t timestamp, char *output, size_t size) {
    if (!output || size == 0) {
        return;
    }
    
    struct tm *tm_info = localtime(&timestamp);
    if (tm_info) {
        strftime(output, size, "%Y-%m-%d", tm_info);
    } else {
        output[0] = '\0';
    }
}

time_t get_date_only(time_t timestamp) {
    struct tm *tm_info = localtime(&timestamp);
    if (tm_info) {
        tm_info->tm_hour = 0;
        tm_info->tm_min = 0;
        tm_info->tm_sec = 0;
        return mktime(tm_info);
    }
    return timestamp;
}
