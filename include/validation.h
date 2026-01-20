#ifndef VALIDATION_H
#define VALIDATION_H

#include "common.h"

// Email validation
bool validate_email(const char *email);

// Password validation (minimum length check)
bool validate_password(const char *password, int min_length);

// Required field validation
bool validate_required(const char *value);

// Date validation
bool validate_date(const char *date_str);

// Time validation
bool validate_time(const char *time_str);

// Date and time validation combined
bool validate_datetime(const char *datetime_str);

// Roll number validation
bool validate_roll_number(const char *roll_number);

// Phone number validation
bool validate_phone(const char *phone);

// Parse date string to time_t
bool parse_datetime(const char *datetime_str, time_t *result);

// Format time_t to string
void format_datetime(time_t timestamp, char *output, size_t size);

// Format date only
void format_date(time_t timestamp, char *output, size_t size);

// Get current date at midnight
time_t get_date_only(time_t timestamp);

#endif // VALIDATION_H
