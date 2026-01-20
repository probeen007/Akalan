#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "security.h"

// Database initialization and cleanup
bool db_init(const char *db_path);
void db_cleanup(void);

// Database schema creation
bool db_create_tables(void);

// User operations
bool db_create_user(const char *email, const char *password_hash, const char *name);
bool db_user_exists(const char *email);
bool db_verify_user(const char *email, const char *password, int *user_id, char *name);

// Class operations
bool db_create_class(const char *name, const char *description, int teacher_id, int *class_id);
bool db_update_class(int id, const char *name, const char *description);
bool db_delete_class(int id);
Class** db_get_user_classes(int teacher_id, int *count);
Class* db_get_class_by_id(int id);
void db_free_classes(Class **classes, int count);

// Student operations
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char roll_number[50];
    char phone[20];
    int class_id;
    time_t created_at;
} Student;

bool db_create_student(const char *name, const char *email, const char *roll_number, const char *phone, int class_id);
bool db_update_student(int id, const char *name, const char *email, const char *roll_number, const char *phone);
bool db_delete_student(int id);
Student** db_get_all_students(int *count);
Student** db_get_students_by_class(int class_id, int *count);
Student* db_get_student_by_id(int id);
void db_free_students(Student **students, int count);
bool db_student_email_exists(const char *email, int exclude_id);
bool db_student_roll_exists(const char *roll_number, int exclude_id);

// Assignment operations
typedef struct {
    int id;
    char title[MAX_TITLE_LENGTH];
    char subject[MAX_SUBJECT_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    time_t due_date;
    int created_by;
    int class_id;
    time_t created_at;
} Assignment;

bool db_create_assignment(const char *title, const char *subject, const char *description, time_t due_date, int created_by, int class_id);
bool db_update_assignment(int id, const char *title, const char *subject, const char *description, time_t due_date);
bool db_delete_assignment(int id);
Assignment** db_get_all_assignments(int *count);
Assignment** db_get_assignments_by_class(int class_id, int *count);
Assignment* db_get_assignment_by_id(int id);
void db_free_assignments(Assignment **assignments, int count);

// Assignment submission operations
typedef struct {
    int id;
    int assignment_id;
    int student_id;
    SubmissionStatus status;
    QualityAssessment quality;
    time_t submitted_at;
    char notes[MAX_DESCRIPTION_LENGTH];
} AssignmentSubmission;

bool db_create_or_update_submission(int assignment_id, int student_id, SubmissionStatus status, QualityAssessment quality, const char *notes);
AssignmentSubmission** db_get_submissions_by_assignment(int assignment_id, int *count);
AssignmentSubmission* db_get_submission(int assignment_id, int student_id);
void db_free_submissions(AssignmentSubmission **submissions, int count);

// Attendance operations
typedef struct {
    int id;
    int student_id;
    time_t date;
    AttendanceStatus status;
    char notes[MAX_DESCRIPTION_LENGTH];
} Attendance;

bool db_mark_attendance(int student_id, time_t date, AttendanceStatus status, const char *notes);
Attendance** db_get_attendance_by_date(time_t date, int *count);
Attendance** db_get_attendance_by_student(int student_id, int *count);
void db_free_attendance(Attendance **attendance, int count);

// Database migration
bool db_migrate_to_class_system(int teacher_id);

#endif // DATABASE_H
