#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <gtk/gtk.h>
#include <sqlite3.h>

// Common constants
#define MAX_STRING_LENGTH 256
#define MAX_EMAIL_LENGTH 100
#define MAX_PASSWORD_LENGTH 128
#define MAX_NAME_LENGTH 100
#define MAX_SUBJECT_LENGTH 100
#define MAX_TITLE_LENGTH 200
#define MAX_DESCRIPTION_LENGTH 1000

// Assignment submission status
typedef enum {
    SUBMISSION_NONE = 0,
    SUBMISSION_TIMELY = 1,
    SUBMISSION_LATE = 2
} SubmissionStatus;

// Assignment quality assessment
typedef enum {
    QUALITY_POOR = 0,
    QUALITY_BELOW_AVERAGE = 1,
    QUALITY_ABOVE_AVERAGE = 2,
    QUALITY_HIGH = 3
} QualityAssessment;

// Attendance status
typedef enum {
    ATTENDANCE_ABSENT = 0,
    ATTENDANCE_PRESENT = 1,
    ATTENDANCE_LATE = 2
} AttendanceStatus;

// Class structure
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    int teacher_id;
} Class;

// User session structure
typedef struct {
    int user_id;
    char email[MAX_EMAIL_LENGTH];
    char name[MAX_NAME_LENGTH];
    bool is_logged_in;
    int current_class_id;  // Currently selected class
} UserSession;

// Global user session
extern UserSession g_session;

// Global database connection
extern sqlite3 *g_db;

// Function to show error dialog
void show_error_dialog(GtkWindow *parent, const char *message);

// Function to show info dialog
void show_info_dialog(GtkWindow *parent, const char *message);

// Function to show confirmation dialog
bool show_confirm_dialog(GtkWindow *parent, const char *message);

#endif // COMMON_H
