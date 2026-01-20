#include "database.h"
#include "security.h"

bool db_init(const char *db_path) {
    int rc = sqlite3_open(db_path, &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(g_db));
        return false;
    }
    
    // Enable foreign keys
    char *err_msg = NULL;
    rc = sqlite3_exec(g_db, "PRAGMA foreign_keys = ON;", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to enable foreign keys: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    return db_create_tables();
}

void db_cleanup(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

bool db_create_tables(void) {
    char *err_msg = NULL;
    
    const char *sql_users = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "email TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    const char *sql_classes = 
        "CREATE TABLE IF NOT EXISTS classes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "teacher_id INTEGER NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (teacher_id) REFERENCES users(id)"
        ");";
    
    const char *sql_students = 
        "CREATE TABLE IF NOT EXISTS students ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "roll_number TEXT UNIQUE NOT NULL,"
        "phone TEXT,"
        "class_id INTEGER NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (class_id) REFERENCES classes(id) ON DELETE CASCADE"
        ");";
    
    const char *sql_assignments = 
        "CREATE TABLE IF NOT EXISTS assignments ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "subject TEXT NOT NULL,"
        "description TEXT,"
        "due_date DATETIME NOT NULL,"
        "created_by INTEGER NOT NULL,"
        "class_id INTEGER NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (created_by) REFERENCES users(id),"
        "FOREIGN KEY (class_id) REFERENCES classes(id) ON DELETE CASCADE"
        ");";
    
    const char *sql_submissions = 
        "CREATE TABLE IF NOT EXISTS assignment_submissions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "assignment_id INTEGER NOT NULL,"
        "student_id INTEGER NOT NULL,"
        "status INTEGER NOT NULL DEFAULT 0,"
        "quality INTEGER NOT NULL DEFAULT 0,"
        "submitted_at DATETIME,"
        "notes TEXT,"
        "FOREIGN KEY (assignment_id) REFERENCES assignments(id) ON DELETE CASCADE,"
        "FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,"
        "UNIQUE(assignment_id, student_id)"
        ");";
    
    const char *sql_attendance = 
        "CREATE TABLE IF NOT EXISTS attendance ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER NOT NULL,"
        "date DATE NOT NULL,"
        "status INTEGER NOT NULL DEFAULT 0,"
        "notes TEXT,"
        "FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,"
        "UNIQUE(student_id, date)"
        ");";
    
    const char *tables[] = {sql_users, sql_classes, sql_students, sql_assignments, sql_submissions, sql_attendance};
    
    for (int i = 0; i < 6; i++) {
        int rc = sqlite3_exec(g_db, tables[i], NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            return false;
        }
    }
    
    // Create indexes for performance
    const char *indexes[] = {
        "CREATE INDEX IF NOT EXISTS idx_students_class_id ON students(class_id);",
        "CREATE INDEX IF NOT EXISTS idx_students_email ON students(email);",
        "CREATE INDEX IF NOT EXISTS idx_students_roll ON students(roll_number);",
        "CREATE INDEX IF NOT EXISTS idx_assignments_class_id ON assignments(class_id);",
        "CREATE INDEX IF NOT EXISTS idx_attendance_student_id ON attendance(student_id);",
        "CREATE INDEX IF NOT EXISTS idx_attendance_date ON attendance(date);",
        "CREATE INDEX IF NOT EXISTS idx_submissions_assignment_id ON assignment_submissions(assignment_id);",
        "CREATE INDEX IF NOT EXISTS idx_submissions_student_id ON assignment_submissions(student_id);",
        "CREATE INDEX IF NOT EXISTS idx_classes_teacher_id ON classes(teacher_id);"
    };
    
    for (int i = 0; i < 9; i++) {
        int rc = sqlite3_exec(g_db, indexes[i], NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Index creation error: %s\n", err_msg);
            sqlite3_free(err_msg);
            // Continue anyway - indexes are optimization, not critical
        }
    }
    
    return true;
}

// User operations
bool db_create_user(const char *email, const char *password_hash, const char *name) {
    if (!g_db) return false;
    if (!email || !password_hash || !name) return false;
    
    const char *sql = "INSERT INTO users (email, password_hash, name) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, name, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_user_exists(const char *email) {
    if (!g_db || !email) return false;
    
    const char *sql = "SELECT COUNT(*) FROM users WHERE email = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count > 0;
}

bool db_verify_user(const char *email, const char *password, int *user_id, char *name) {
    if (!g_db || !email || !password || !user_id || !name) return false;
    
    const char *sql = "SELECT id, name, password_hash FROM users WHERE email = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    
    bool verified = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_hash = (const char *)sqlite3_column_text(stmt, 2);
        // Use verify_password which handles salt extraction and comparison
        if (verify_password(password, stored_hash)) {
            *user_id = sqlite3_column_int(stmt, 0);
            const char *user_name = (const char *)sqlite3_column_text(stmt, 1);
            strncpy(name, user_name, MAX_NAME_LENGTH - 1);
            name[MAX_NAME_LENGTH - 1] = '\0';
            verified = true;
        }
    }
    
    sqlite3_finalize(stmt);
    return verified;
}

// Class operations
bool db_create_class(const char *name, const char *description, int teacher_id, int *class_id) {
    if (!g_db || !name || strlen(name) == 0 || !class_id) {
        return false;
    }
    
    const char *sql = "INSERT INTO classes (name, description, teacher_id) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description ? description : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, teacher_id);
    
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE && class_id) {
        *class_id = (int)sqlite3_last_insert_rowid(g_db);
    }
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_update_class(int id, const char *name, const char *description) {
    if (!g_db || !name || strlen(name) == 0) {
        return false;
    }
    
    const char *sql = "UPDATE classes SET name = ?, description = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description ? description : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_delete_class(int id) {
    if (!g_db || id <= 0) return false;
    
    const char *sql = "DELETE FROM classes WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

Class** db_get_user_classes(int teacher_id, int *count) {
    if (!g_db || !count) return NULL;
    
    const char *sql = "SELECT id, name, description, teacher_id FROM classes WHERE teacher_id = ? ORDER BY created_at DESC;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, teacher_id);
    
    // Count rows first
    int capacity = 10;
    Class **classes = malloc(sizeof(Class *) * capacity);
    if (!classes) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            if (capacity > 10000) {  // Safety limit
                break;
            }
            Class **temp = realloc(classes, sizeof(Class *) * capacity);
            if (!temp) {
                // Free already allocated classes
                for (int i = 0; i < *count; i++) {
                    free(classes[i]);
                }
                free(classes);
                sqlite3_finalize(stmt);
                *count = 0;
                return NULL;
            }
            classes = temp;
        }
        
        Class *cls = malloc(sizeof(Class));
        if (!cls) {
            // Free already allocated classes
            for (int i = 0; i < *count; i++) {
                free(classes[i]);
            }
            free(classes);
            sqlite3_finalize(stmt);
            *count = 0;
            return NULL;
        }
        
        cls->id = sqlite3_column_int(stmt, 0);
        strncpy(cls->name, (const char *)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH - 1);
        cls->name[MAX_NAME_LENGTH - 1] = '\0';
        
        const char *desc = (const char *)sqlite3_column_text(stmt, 2);
        if (desc) {
            strncpy(cls->description, desc, MAX_DESCRIPTION_LENGTH - 1);
            cls->description[MAX_DESCRIPTION_LENGTH - 1] = '\0';
        } else {
            cls->description[0] = '\0';
        }
        
        cls->teacher_id = sqlite3_column_int(stmt, 3);
        
        classes[(*count)++] = cls;
    }
    
    sqlite3_finalize(stmt);
    
    if (*count == 0) {
        free(classes);
        return NULL;
    }
    
    return classes;
}

Class* db_get_class_by_id(int id) {
    if (!g_db || id <= 0) return NULL;
    
    const char *sql = "SELECT id, name, description, teacher_id FROM classes WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    Class *cls = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cls = malloc(sizeof(Class));
        cls->id = sqlite3_column_int(stmt, 0);
        strncpy(cls->name, (const char *)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH - 1);
        cls->name[MAX_NAME_LENGTH - 1] = '\0';
        
        const char *desc = (const char *)sqlite3_column_text(stmt, 2);
        if (desc) {
            strncpy(cls->description, desc, MAX_DESCRIPTION_LENGTH - 1);
            cls->description[MAX_DESCRIPTION_LENGTH - 1] = '\0';
        } else {
            cls->description[0] = '\0';
        }
        
        cls->teacher_id = sqlite3_column_int(stmt, 3);
    }
    
    sqlite3_finalize(stmt);
    return cls;
}

void db_free_classes(Class **classes, int count) {
    if (!classes) return;
    
    for (int i = 0; i < count; i++) {
        free(classes[i]);
    }
    free(classes);
}

// Student operations
bool db_create_student(const char *name, const char *email, const char *roll_number, const char *phone, int class_id) {
    if (!g_db || !name || !email || !roll_number || class_id <= 0) {
        return false;
    }
    
    const char *sql = "INSERT INTO students (name, email, roll_number, phone, class_id) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, roll_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, phone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, class_id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_update_student(int id, const char *name, const char *email, const char *roll_number, const char *phone) {
    if (!g_db || !name || !email || !roll_number || id <= 0) return false;
    
    const char *sql = "UPDATE students SET name = ?, email = ?, roll_number = ?, phone = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, roll_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, phone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_delete_student(int id) {
    if (!g_db || id <= 0) return false;
    
    const char *sql = "DELETE FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

Student** db_get_all_students(int *count) {
    if (!g_db || !count) return NULL;
    
    const char *sql = "SELECT id, name, email, roll_number, phone, class_id, strftime('%s', created_at) FROM students ORDER BY name;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    // Count rows first
    int capacity = 10;
    Student **students = malloc(capacity * sizeof(Student*));
    if (!students) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            if (capacity > 10000) {  // Safety limit
                break;
            }
            Student **temp = realloc(students, capacity * sizeof(Student*));
            if (!temp) {
                db_free_students(students, *count);
                sqlite3_finalize(stmt);
                *count = 0;
                return NULL;
            }
            students = temp;
        }
        
        Student *s = malloc(sizeof(Student));
        if (!s) {
            db_free_students(students, *count);
            sqlite3_finalize(stmt);
            *count = 0;
            return NULL;
        }
        
        s->id = sqlite3_column_int(stmt, 0);
        strncpy(s->name, (const char *)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH - 1);
        s->name[MAX_NAME_LENGTH - 1] = '\0';
        strncpy(s->email, (const char *)sqlite3_column_text(stmt, 2), MAX_EMAIL_LENGTH - 1);
        s->email[MAX_EMAIL_LENGTH - 1] = '\0';
        strncpy(s->roll_number, (const char *)sqlite3_column_text(stmt, 3), 49);
        s->roll_number[49] = '\0';
        
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        if (phone) {
            strncpy(s->phone, (const char *)phone, 19);
            s->phone[19] = '\0';
        } else {
            s->phone[0] = '\0';
        }
        
        s->class_id = sqlite3_column_int(stmt, 5);
        s->created_at = (time_t)sqlite3_column_int64(stmt, 6);
        
        students[*count] = s;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return students;
}

Student** db_get_students_by_class(int class_id, int *count) {
    if (!g_db || !count || class_id <= 0) return NULL;
    
    const char *sql = "SELECT id, name, email, roll_number, phone, class_id, strftime('%s', created_at) FROM students WHERE class_id = ? ORDER BY name;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, class_id);
    
    int capacity = 10;
    Student **students = malloc(capacity * sizeof(Student*));
    if (!students) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            if (capacity > 10000) {  // Safety limit
                break;
            }
            Student **temp = realloc(students, capacity * sizeof(Student*));
            if (!temp) {
                db_free_students(students, *count);
                sqlite3_finalize(stmt);
                *count = 0;
                return NULL;
            }
            students = temp;
        }
        
        Student *s = malloc(sizeof(Student));
        if (!s) {
            db_free_students(students, *count);
            sqlite3_finalize(stmt);
            *count = 0;
            return NULL;
        }
        
        s->id = sqlite3_column_int(stmt, 0);
        strncpy(s->name, (const char *)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH - 1);
        s->name[MAX_NAME_LENGTH - 1] = '\0';
        strncpy(s->email, (const char *)sqlite3_column_text(stmt, 2), MAX_EMAIL_LENGTH - 1);
        s->email[MAX_EMAIL_LENGTH - 1] = '\0';
        strncpy(s->roll_number, (const char *)sqlite3_column_text(stmt, 3), 49);
        s->roll_number[49] = '\0';
        
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        if (phone) {
            strncpy(s->phone, (const char *)phone, 19);
            s->phone[19] = '\0';
        } else {
            s->phone[0] = '\0';
        }
        
        s->class_id = sqlite3_column_int(stmt, 5);
        s->created_at = (time_t)sqlite3_column_int64(stmt, 6);
        
        students[*count] = s;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    
    if (*count == 0) {
        free(students);
        return NULL;
    }
    
    return students;
}

Student* db_get_student_by_id(int id) {
    if (!g_db || id <= 0) return NULL;
    
    const char *sql = "SELECT id, name, email, roll_number, phone, strftime('%s', created_at) FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    Student *s = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        s = malloc(sizeof(Student));
        if (s) {
            s->id = sqlite3_column_int(stmt, 0);
            strncpy(s->name, (const char *)sqlite3_column_text(stmt, 1), MAX_NAME_LENGTH - 1);
            s->name[MAX_NAME_LENGTH - 1] = '\0';
            strncpy(s->email, (const char *)sqlite3_column_text(stmt, 2), MAX_EMAIL_LENGTH - 1);
            s->email[MAX_EMAIL_LENGTH - 1] = '\0';
            strncpy(s->roll_number, (const char *)sqlite3_column_text(stmt, 3), 49);
            s->roll_number[49] = '\0';
            
            const unsigned char *phone = sqlite3_column_text(stmt, 4);
            if (phone) {
                strncpy(s->phone, (const char *)phone, 19);
                s->phone[19] = '\0';
            } else {
                s->phone[0] = '\0';
            }
            
            s->created_at = (time_t)sqlite3_column_int64(stmt, 5);
        }
    }
    
    sqlite3_finalize(stmt);
    return s;
}

void db_free_students(Student **students, int count) {
    if (students) {
        for (int i = 0; i < count; i++) {
            free(students[i]);
        }
        free(students);
    }
}

// Check if student email exists (for duplicate detection)
bool db_student_email_exists(const char *email, int exclude_id) {
    if (!g_db || !email) return false;
    
    const char *sql = "SELECT COUNT(*) FROM students WHERE email = ? AND id != ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, exclude_id);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count > 0;
}

// Check if student roll number exists (for duplicate detection)
bool db_student_roll_exists(const char *roll_number, int exclude_id) {
    if (!g_db || !roll_number) return false;
    
    const char *sql = "SELECT COUNT(*) FROM students WHERE roll_number = ? AND id != ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, roll_number, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, exclude_id);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count > 0;
}

// Assignment operations
bool db_create_assignment(const char *title, const char *subject, const char *description, time_t due_date, int created_by, int class_id) {
    if (!g_db || !title || !subject || class_id <= 0) {
        fprintf(stderr, "Error: Invalid class_id\n");
        return false;
    }
    
    const char *sql = "INSERT INTO assignments (title, subject, description, due_date, created_by, class_id) VALUES (?, ?, ?, datetime(?, 'unixepoch'), ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, subject, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, description, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)due_date);
    sqlite3_bind_int(stmt, 5, created_by);
    sqlite3_bind_int(stmt, 6, class_id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_update_assignment(int id, const char *title, const char *subject, const char *description, time_t due_date) {
    if (!g_db || !title || !subject || id <= 0) return false;
    
    const char *sql = "UPDATE assignments SET title = ?, subject = ?, description = ?, due_date = datetime(?, 'unixepoch') WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, subject, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, description, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)due_date);
    sqlite3_bind_int(stmt, 5, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_delete_assignment(int id) {
    if (!g_db || id <= 0) return false;
    
    const char *sql = "DELETE FROM assignments WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

Assignment** db_get_all_assignments(int *count) {
    if (!g_db || !count) return NULL;
    
    const char *sql = "SELECT id, title, subject, description, strftime('%s', due_date), created_by, strftime('%s', created_at) FROM assignments ORDER BY due_date DESC;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    int capacity = 10;
    Assignment **assignments = malloc(capacity * sizeof(Assignment*));
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            assignments = realloc(assignments, capacity * sizeof(Assignment*));
        }
        
        Assignment *a = malloc(sizeof(Assignment));
        a->id = sqlite3_column_int(stmt, 0);
        strncpy(a->title, (const char *)sqlite3_column_text(stmt, 1), MAX_TITLE_LENGTH - 1);
        strncpy(a->subject, (const char *)sqlite3_column_text(stmt, 2), MAX_SUBJECT_LENGTH - 1);
        
        const unsigned char *desc = sqlite3_column_text(stmt, 3);
        if (desc) {
            strncpy(a->description, (const char *)desc, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            a->description[0] = '\0';
        }
        
        a->due_date = (time_t)sqlite3_column_int64(stmt, 4);
        a->created_by = sqlite3_column_int(stmt, 5);
        a->created_at = (time_t)sqlite3_column_int64(stmt, 6);
        
        assignments[*count] = a;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return assignments;
}

Assignment** db_get_assignments_by_class(int class_id, int *count) {
    const char *sql = "SELECT id, title, subject, description, strftime('%s', due_date), created_by, strftime('%s', created_at) FROM assignments WHERE class_id = ? ORDER BY due_date DESC;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, class_id);
    
    int capacity = 10;
    Assignment **assignments = malloc(capacity * sizeof(Assignment*));
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            assignments = realloc(assignments, capacity * sizeof(Assignment*));
        }
        
        Assignment *a = malloc(sizeof(Assignment));
        a->id = sqlite3_column_int(stmt, 0);
        strncpy(a->title, (const char *)sqlite3_column_text(stmt, 1), MAX_TITLE_LENGTH - 1);
        strncpy(a->subject, (const char *)sqlite3_column_text(stmt, 2), MAX_SUBJECT_LENGTH - 1);
        
        const unsigned char *desc = sqlite3_column_text(stmt, 3);
        if (desc) {
            strncpy(a->description, (const char *)desc, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            a->description[0] = '\0';
        }
        
        a->due_date = (time_t)sqlite3_column_int64(stmt, 4);
        a->created_by = sqlite3_column_int(stmt, 5);
        a->created_at = (time_t)sqlite3_column_int64(stmt, 6);
        a->class_id = class_id;
        
        assignments[*count] = a;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return assignments;
}

Assignment* db_get_assignment_by_id(int id) {
    const char *sql = "SELECT id, title, subject, description, strftime('%s', due_date), created_by, strftime('%s', created_at) FROM assignments WHERE id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    Assignment *a = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        a = malloc(sizeof(Assignment));
        a->id = sqlite3_column_int(stmt, 0);
        strncpy(a->title, (const char *)sqlite3_column_text(stmt, 1), MAX_TITLE_LENGTH - 1);
        strncpy(a->subject, (const char *)sqlite3_column_text(stmt, 2), MAX_SUBJECT_LENGTH - 1);
        
        const unsigned char *desc = sqlite3_column_text(stmt, 3);
        if (desc) {
            strncpy(a->description, (const char *)desc, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            a->description[0] = '\0';
        }
        
        a->due_date = (time_t)sqlite3_column_int64(stmt, 4);
        a->created_by = sqlite3_column_int(stmt, 5);
        a->created_at = (time_t)sqlite3_column_int64(stmt, 6);
    }
    
    sqlite3_finalize(stmt);
    return a;
}

void db_free_assignments(Assignment **assignments, int count) {
    if (assignments) {
        for (int i = 0; i < count; i++) {
            free(assignments[i]);
        }
        free(assignments);
    }
}

// Assignment submission operations
bool db_create_or_update_submission(int assignment_id, int student_id, SubmissionStatus status, QualityAssessment quality, const char *notes) {
    const char *sql = "INSERT OR REPLACE INTO assignment_submissions (assignment_id, student_id, status, quality, submitted_at, notes) "
                     "VALUES (?, ?, ?, ?, datetime('now'), ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, assignment_id);
    sqlite3_bind_int(stmt, 2, student_id);
    sqlite3_bind_int(stmt, 3, (int)status);
    sqlite3_bind_int(stmt, 4, (int)quality);
    sqlite3_bind_text(stmt, 5, notes ? notes : "", -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

AssignmentSubmission** db_get_submissions_by_assignment(int assignment_id, int *count) {
    const char *sql = "SELECT id, assignment_id, student_id, status, quality, strftime('%s', submitted_at), notes "
                     "FROM assignment_submissions WHERE assignment_id = ?;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, assignment_id);
    
    int capacity = 10;
    AssignmentSubmission **submissions = malloc(capacity * sizeof(AssignmentSubmission*));
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            submissions = realloc(submissions, capacity * sizeof(AssignmentSubmission*));
        }
        
        AssignmentSubmission *s = malloc(sizeof(AssignmentSubmission));
        s->id = sqlite3_column_int(stmt, 0);
        s->assignment_id = sqlite3_column_int(stmt, 1);
        s->student_id = sqlite3_column_int(stmt, 2);
        s->status = (SubmissionStatus)sqlite3_column_int(stmt, 3);
        s->quality = (QualityAssessment)sqlite3_column_int(stmt, 4);
        s->submitted_at = (time_t)sqlite3_column_int64(stmt, 5);
        
        const unsigned char *notes = sqlite3_column_text(stmt, 6);
        if (notes) {
            strncpy(s->notes, (const char *)notes, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            s->notes[0] = '\0';
        }
        
        submissions[*count] = s;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return submissions;
}

AssignmentSubmission* db_get_submission(int assignment_id, int student_id) {
    const char *sql = "SELECT id, assignment_id, student_id, status, quality, strftime('%s', submitted_at), notes "
                     "FROM assignment_submissions WHERE assignment_id = ? AND student_id = ?;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, assignment_id);
    sqlite3_bind_int(stmt, 2, student_id);
    
    AssignmentSubmission *s = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        s = malloc(sizeof(AssignmentSubmission));
        s->id = sqlite3_column_int(stmt, 0);
        s->assignment_id = sqlite3_column_int(stmt, 1);
        s->student_id = sqlite3_column_int(stmt, 2);
        s->status = (SubmissionStatus)sqlite3_column_int(stmt, 3);
        s->quality = (QualityAssessment)sqlite3_column_int(stmt, 4);
        s->submitted_at = (time_t)sqlite3_column_int64(stmt, 5);
        
        const unsigned char *notes = sqlite3_column_text(stmt, 6);
        if (notes) {
            strncpy(s->notes, (const char *)notes, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            s->notes[0] = '\0';
        }
    }
    
    sqlite3_finalize(stmt);
    return s;
}

void db_free_submissions(AssignmentSubmission **submissions, int count) {
    if (submissions) {
        for (int i = 0; i < count; i++) {
            free(submissions[i]);
        }
        free(submissions);
    }
}

// Attendance operations
bool db_mark_attendance(int student_id, time_t date, AttendanceStatus status, const char *notes) {
    const char *sql = "INSERT OR REPLACE INTO attendance (student_id, date, status, notes) "
                     "VALUES (?, date(?, 'unixepoch'), ?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)date);
    sqlite3_bind_int(stmt, 3, (int)status);
    sqlite3_bind_text(stmt, 4, notes ? notes : "", -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

Attendance** db_get_attendance_by_date(time_t date, int *count) {
    const char *sql = "SELECT id, student_id, strftime('%s', date), status, notes "
                     "FROM attendance WHERE date = date(?, 'unixepoch');";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)date);
    
    int capacity = 10;
    Attendance **attendance = malloc(capacity * sizeof(Attendance*));
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            attendance = realloc(attendance, capacity * sizeof(Attendance*));
        }
        
        Attendance *a = malloc(sizeof(Attendance));
        a->id = sqlite3_column_int(stmt, 0);
        a->student_id = sqlite3_column_int(stmt, 1);
        a->date = (time_t)sqlite3_column_int64(stmt, 2);
        a->status = (AttendanceStatus)sqlite3_column_int(stmt, 3);
        
        const unsigned char *notes = sqlite3_column_text(stmt, 4);
        if (notes) {
            strncpy(a->notes, (const char *)notes, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            a->notes[0] = '\0';
        }
        
        attendance[*count] = a;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return attendance;
}

Attendance** db_get_attendance_by_student(int student_id, int *count) {
    const char *sql = "SELECT id, student_id, strftime('%s', date), status, notes "
                     "FROM attendance WHERE student_id = ? ORDER BY date DESC;";
    sqlite3_stmt *stmt;
    
    *count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, student_id);
    
    int capacity = 10;
    Attendance **attendance = malloc(capacity * sizeof(Attendance*));
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            attendance = realloc(attendance, capacity * sizeof(Attendance*));
        }
        
        Attendance *a = malloc(sizeof(Attendance));
        a->id = sqlite3_column_int(stmt, 0);
        a->student_id = sqlite3_column_int(stmt, 1);
        a->date = (time_t)sqlite3_column_int64(stmt, 2);
        a->status = (AttendanceStatus)sqlite3_column_int(stmt, 3);
        
        const unsigned char *notes = sqlite3_column_text(stmt, 4);
        if (notes) {
            strncpy(a->notes, (const char *)notes, MAX_DESCRIPTION_LENGTH - 1);
        } else {
            a->notes[0] = '\0';
        }
        
        attendance[*count] = a;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return attendance;
}

void db_free_attendance(Attendance **attendance, int count) {
    if (attendance) {
        for (int i = 0; i < count; i++) {
            free(attendance[i]);
        }
        free(attendance);
    }
}

// Database migration for multi-class support
bool db_migrate_to_class_system(int teacher_id) {
    char *err_msg = NULL;
    
    printf("\n=== Starting migration for teacher_id: %d ===\n", teacher_id);
    
    // Check if there are ANY students in the database
    const char *count_students_sql = "SELECT COUNT(*) FROM students;";
    sqlite3_stmt *stmt;
    int total_students = 0;
    
    if (sqlite3_prepare_v2(g_db, count_students_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_students = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    printf("Total students in database: %d\n", total_students);
    
    // Check if there are ANY assignments
    const char *count_assignments_sql = "SELECT COUNT(*) FROM assignments;";
    int total_assignments = 0;
    
    if (sqlite3_prepare_v2(g_db, count_assignments_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_assignments = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    printf("Total assignments in database: %d\n", total_assignments);
    
    // If there's existing data but no classes, create Test Class and migrate
    if (total_students > 0 || total_assignments > 0) {
        // Check if this teacher already has classes
        const char *count_classes_sql = "SELECT COUNT(*) FROM classes WHERE teacher_id = ?;";
        int teacher_classes = 0;
        
        if (sqlite3_prepare_v2(g_db, count_classes_sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, teacher_id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                teacher_classes = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        
        printf("Teacher has %d existing classes\n", teacher_classes);
        
        // Check for orphaned students (without class_id)
        const char *count_orphans_sql = "SELECT COUNT(*) FROM students WHERE class_id IS NULL OR class_id = 0;";
        int orphaned_students = 0;
        
        if (sqlite3_prepare_v2(g_db, count_orphans_sql, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                orphaned_students = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        
        printf("Found %d orphaned students\n", orphaned_students);
        
        // Check for orphaned assignments
        const char *count_orphan_assign_sql = "SELECT COUNT(*) FROM assignments WHERE class_id IS NULL OR class_id = 0;";
        int orphaned_assignments = 0;
        
        if (sqlite3_prepare_v2(g_db, count_orphan_assign_sql, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                orphaned_assignments = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        
        printf("Found %d orphaned assignments\n", orphaned_assignments);
        
        // If there are orphaned records, migrate them
        if (orphaned_students > 0 || orphaned_assignments > 0) {
            int default_class_id = 0;
            
            // If no classes exist, create Test Class, otherwise use first class
            if (teacher_classes == 0) {
                printf("Creating 'Test Class' for existing data migration...\n");
                
                // Create default class
                if (!db_create_class("Test Class", "Migrated data from previous version", teacher_id, &default_class_id)) {
                    fprintf(stderr, "Failed to create default class for migration\n");
                    return false;
                }
                
                printf("Created Test Class with ID: %d\n", default_class_id);
            } else {
                // Get the first class ID
                const char *get_first_class_sql = "SELECT id FROM classes WHERE teacher_id = ? ORDER BY id LIMIT 1;";
                if (sqlite3_prepare_v2(g_db, get_first_class_sql, -1, &stmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_int(stmt, 1, teacher_id);
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        default_class_id = sqlite3_column_int(stmt, 0);
                    }
                    sqlite3_finalize(stmt);
                }
                printf("Using existing class ID %d for orphaned data\n", default_class_id);
            }
            
            // Migrate orphaned students
            if (orphaned_students > 0) {
                char update_students[200];
                snprintf(update_students, sizeof(update_students),
                        "UPDATE students SET class_id = %d WHERE class_id IS NULL OR class_id = 0;",
                        default_class_id);
                
                if (sqlite3_exec(g_db, update_students, NULL, NULL, &err_msg) != SQLITE_OK) {
                    fprintf(stderr, "Failed to migrate students: %s\n", err_msg);
                    sqlite3_free(err_msg);
                    return false;
                }
                printf("Migrated %d orphaned students to class %d\n", orphaned_students, default_class_id);
            }
            
            // Migrate orphaned assignments  
            if (orphaned_assignments > 0) {
                char update_assignments[200];
                snprintf(update_assignments, sizeof(update_assignments),
                        "UPDATE assignments SET class_id = %d WHERE class_id IS NULL OR class_id = 0;",
                        default_class_id);
                
                if (sqlite3_exec(g_db, update_assignments, NULL, NULL, &err_msg) != SQLITE_OK) {
                    fprintf(stderr, "Failed to migrate assignments: %s\n", err_msg);
                    sqlite3_free(err_msg);
                    return false;
                }
                printf("Migrated %d orphaned assignments to class %d\n", orphaned_assignments, default_class_id);
            }
            
            printf("Migration completed successfully!\n");
        } else {
            printf("Classes already exist - no migration needed\n");
        }
    } else {
        printf("No existing data found - no migration needed\n");
    }
    
    printf("=== Migration check complete ===\n\n");
    return true;
}
