/**
 * UI Attendance Module - View and Mark Student Attendance
 * Modern UI with date list, attendance sheets, and search functionality
 */

#include <gtk/gtk.h>
#include <time.h>
#include <string.h>
#include "database.h"
#include "common.h"
#include "validation.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *date_listbox;
    GtkWidget *attendance_grid;
    GtkWidget *search_entry;
    GtkWidget *stats_label;
    GtkWidget *mark_date_entry;
    GtkWidget *mark_attendance_grid;
    GtkWidget *mark_stats_label;
    GtkWidget *notebook;
    char selected_date[20];
} AttendanceWindow;

// Forward declarations
static void load_attendance_dates(AttendanceWindow *aw);
static void show_attendance_for_date(AttendanceWindow *aw, const char *date);
static void on_date_row_activated(GtkListBox *listbox, GtkListBoxRow *row, gpointer user_data);
static void on_search_changed(GtkWidget *widget, gpointer user_data);
static void on_clear_search(GtkWidget *widget, gpointer user_data);
static void on_delete_attendance_clicked(GtkButton *button, gpointer user_data);
static void on_mark_attendance_clicked(GtkButton *button, gpointer user_data);
static void refresh_mark_attendance_list(AttendanceWindow *aw);
static const char* get_day_of_week(const char *date);

// CSS Styling for modern UI
static void apply_attendance_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        ".attendance-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 15px;"
        "  border-radius: 8px;"
        "  margin: 10px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        ".date-item {"
        "  padding: 12px;"
        "  margin: 4px;"
        "  border-radius: 6px;"
        "  background: #f8f9fa;"
        "  transition: background 0.3s ease;"
        "}"
        ".date-item:hover {"
        "  background: #e9ecef;"
        "}"
        ".date-item.selected {"
        "  background: #667eea;"
        "  color: white;"
        "}"
        ".attendance-section {"
        "  background: white;"
        "  border-radius: 8px;"
        "  padding: 15px;"
        "  margin: 10px;"
        "  box-shadow: 0 2px 8px rgba(0,0,0,0.1);"
        "}"
        ".student-row {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #e9ecef;"
        "}"
        ".stats-label {"
        "  font-size: 14px;"
        "  color: #495057;"
        "  padding: 10px;"
        "  background: #e7f3ff;"
        "  border-radius: 6px;"
        "  margin: 10px;"
        "}"
        ".mark-button {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 10px 20px;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  min-width: 150px;"
        "}"
        ".date-entry {"
        "  padding: 8px;"
        "  border-radius: 6px;"
        "  border: 2px solid #667eea;"
        "  min-width: 200px;"
        "}"
        ".date-search-box {"
        "  background: #f8f9fa;"
        "  padding: 15px;"
        "  border-radius: 8px;"
        "  margin: 10px;"
        "}"
        ".button-container {"
        "  padding: 10px;"
        "}";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

// Get day of week from date string (YYYY-MM-DD)
static const char* get_day_of_week(const char *date) {
    static const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
                                  "Thursday", "Friday", "Saturday"};
    struct tm tm = {0};
    if (sscanf(date, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3) {
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        mktime(&tm);
        return days[tm.tm_wday];
    }
    return "Unknown";
}

// Load all attendance dates from database
static void load_attendance_dates(AttendanceWindow *aw) {
    // Clear existing items
    GList *children = gtk_container_get_children(GTK_CONTAINER(aw->date_listbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Get search filter
    const char *search_text = gtk_entry_get_text(GTK_ENTRY(aw->search_entry));
    
    // Query unique dates from attendance table filtered by current class
    extern sqlite3 *g_db;
    if (!g_db) return;
    
    const char *sql = 
        "SELECT DISTINCT a.date FROM attendance a "
        "INNER JOIN students s ON a.student_id = s.id "
        "WHERE s.class_id = ? "
        "ORDER BY a.date DESC";
    sqlite3_stmt *stmt;
    int count = 0;
    GtkWidget *first_row = NULL;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, g_session.current_class_id);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *date = (const char *)sqlite3_column_text(stmt, 0);
            if (!date) continue;
            
            // Apply search filter
            if (search_text && strlen(search_text) > 0) {
                if (strstr(date, search_text) == NULL) {
                    continue;
                }
            }
            
            // Create date row with day of week
            const char *day = get_day_of_week(date);
            char label_text[150];
            snprintf(label_text, sizeof(label_text), "%s\n<small>%s</small>", 
                     date, day);
            
            GtkWidget *label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(label), label_text);
            gtk_widget_set_halign(label, GTK_ALIGN_START);
            
            GtkWidget *row = gtk_list_box_row_new();
            gtk_container_add(GTK_CONTAINER(row), label);
            
            // Store date in row data
            g_object_set_data_full(G_OBJECT(row), "date", 
                                   g_strdup(date), g_free);
            
            // Apply CSS class
            GtkStyleContext *context = gtk_widget_get_style_context(row);
            gtk_style_context_add_class(context, "date-item");
            
            gtk_container_add(GTK_CONTAINER(aw->date_listbox), row);
            
            // Store first row for auto-selection
            if (count == 0) {
                first_row = row;
            }
            count++;
        }
        sqlite3_finalize(stmt);
    }
    
    gtk_widget_show_all(aw->date_listbox);
    
    // Handle search results
    if (search_text && strlen(search_text) > 0) {
        if (count == 0) {
            // No results found for search
            gtk_label_set_text(GTK_LABEL(aw->stats_label), 
                "No attendance records found for the searched date");
            
            // Clear attendance grid
            GList *grid_children = gtk_container_get_children(GTK_CONTAINER(aw->attendance_grid));
            for (GList *iter = grid_children; iter != NULL; iter = g_list_next(iter)) {
                gtk_widget_destroy(GTK_WIDGET(iter->data));
            }
            g_list_free(grid_children);
        } else if (count == 1 && first_row) {
            // Auto-select the single matching result
            gtk_list_box_select_row(GTK_LIST_BOX(aw->date_listbox), GTK_LIST_BOX_ROW(first_row));
            const char *date = g_object_get_data(G_OBJECT(first_row), "date");
            if (date) {
                show_attendance_for_date(aw, date);
            }
        }
    } else if (count > 0 && first_row) {
        // No search, auto-select first (latest) date
        gtk_list_box_select_row(GTK_LIST_BOX(aw->date_listbox), GTK_LIST_BOX_ROW(first_row));
        const char *date = g_object_get_data(G_OBJECT(first_row), "date");
        if (date) {
            show_attendance_for_date(aw, date);
        }
    } else if (count == 0) {
        // No attendance records at all
        gtk_label_set_text(GTK_LABEL(aw->stats_label), 
            "No attendance records available. Use 'Mark Attendance' tab to create records.");
    }
}

// Show attendance sheet for selected date
static void show_attendance_for_date(AttendanceWindow *aw, const char *date) {
    // Store selected date
    strncpy(aw->selected_date, date, sizeof(aw->selected_date) - 1);
    
    // Clear existing attendance grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(aw->attendance_grid));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Add headers
    GtkWidget *header_name = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_name), "<b>Student Name</b>");
    gtk_widget_set_halign(header_name, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->attendance_grid), header_name, 0, 0, 1, 1);
    
    GtkWidget *header_roll = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_roll), "<b>Roll No</b>");
    gtk_widget_set_halign(header_roll, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->attendance_grid), header_roll, 1, 0, 1, 1);
    
    GtkWidget *header_status = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_status), "<b>Status</b>");
    gtk_widget_set_halign(header_status, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->attendance_grid), header_status, 2, 0, 1, 1);
    
    GtkWidget *header_notes = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_notes), "<b>Notes</b>");
    gtk_widget_set_halign(header_notes, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->attendance_grid), header_notes, 3, 0, 1, 1);
    
    // Get attendance data for this date filtered by current class
    extern sqlite3 *g_db;
    if (!g_db) return;
    
    const char *sql = "SELECT s.name, s.roll_number, a.status, a.notes "
                     "FROM students s "
                     "LEFT JOIN attendance a ON s.id = a.student_id AND a.date = ? "
                     "WHERE s.class_id = ? "
                     "ORDER BY s.roll_number";
    
    sqlite3_stmt *stmt;
    int row = 1;
    int present_count = 0;
    int absent_count = 0;
    int late_count = 0;
    int total_count = 0;
    
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, g_session.current_class_id);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            const char *roll = (const char *)sqlite3_column_text(stmt, 1);
            int status = sqlite3_column_type(stmt, 2) != SQLITE_NULL ? 
                         sqlite3_column_int(stmt, 2) : -1;
            const char *notes = (const char *)sqlite3_column_text(stmt, 3);
            
            // Create row widgets
            GtkWidget *name_label = gtk_label_new(name);
            gtk_widget_set_halign(name_label, GTK_ALIGN_START);
            gtk_widget_set_margin_start(name_label, 10);
            
            GtkWidget *roll_label = gtk_label_new(roll);
            gtk_widget_set_halign(roll_label, GTK_ALIGN_START);
            
            GtkWidget *status_label = gtk_label_new(NULL);
            gtk_widget_set_halign(status_label, GTK_ALIGN_START);
            
            if (status == ATTENDANCE_PRESENT) {
                gtk_label_set_markup(GTK_LABEL(status_label), 
                    "<span foreground='green'>Present</span>");
                present_count++;
            } else if (status == ATTENDANCE_ABSENT) {
                gtk_label_set_markup(GTK_LABEL(status_label), 
                    "<span foreground='red'>Absent</span>");
                absent_count++;
            } else if (status == 2) {
                gtk_label_set_markup(GTK_LABEL(status_label), 
                    "<span foreground='orange'>Late</span>");
                late_count++;
            } else {
                gtk_label_set_markup(GTK_LABEL(status_label), 
                    "<span foreground='gray'>Not Marked</span>");
            }
            
            // Add to grid
            gtk_grid_attach(GTK_GRID(aw->attendance_grid), name_label, 0, row, 1, 1);
            gtk_grid_attach(GTK_GRID(aw->attendance_grid), roll_label, 1, row, 1, 1);
            gtk_grid_attach(GTK_GRID(aw->attendance_grid), status_label, 2, row, 1, 1);
            
            // Notes label
            GtkWidget *notes_label = gtk_label_new(notes && strlen(notes) > 0 ? notes : "-");
            gtk_widget_set_halign(notes_label, GTK_ALIGN_START);
            gtk_label_set_line_wrap(GTK_LABEL(notes_label), TRUE);
            gtk_label_set_max_width_chars(GTK_LABEL(notes_label), 40);
            gtk_grid_attach(GTK_GRID(aw->attendance_grid), notes_label, 3, row, 1, 1);
            
            // Apply student-row style
            GtkStyleContext *ctx;
            ctx = gtk_widget_get_style_context(name_label);
            gtk_style_context_add_class(ctx, "student-row");
            
            row++;
            total_count++;
        }
        sqlite3_finalize(stmt);
    }
    
    // Update statistics
    char stats[250];
    snprintf(stats, sizeof(stats), 
             "Attendance Statistics for %s (%s)\n"
             "Total: %d | Present: %d | Absent: %d | Late: %d",
             date, get_day_of_week(date), total_count, present_count, absent_count, late_count);
    gtk_label_set_text(GTK_LABEL(aw->stats_label), stats);
    
    gtk_widget_show_all(aw->attendance_grid);
}

// Date row activated callback
static void on_date_row_activated(GtkListBox *listbox, GtkListBoxRow *row, 
                                  gpointer user_data) {
    AttendanceWindow *aw = (AttendanceWindow *)user_data;
    
    if (!row || !aw) return;
    
    // Remove previous selection
    GList *children = gtk_container_get_children(GTK_CONTAINER(listbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *child = GTK_WIDGET(iter->data);
        GtkStyleContext *ctx = gtk_widget_get_style_context(child);
        gtk_style_context_remove_class(ctx, "selected");
    }
    g_list_free(children);
    
    // Add selection to current row
    GtkStyleContext *ctx = gtk_widget_get_style_context(GTK_WIDGET(row));
    gtk_style_context_add_class(ctx, "selected");
    
    // Get date from row data
    const char *date = g_object_get_data(G_OBJECT(row), "date");
    if (date) {
        show_attendance_for_date(aw, date);
    }
}

// Search changed callback
static void on_search_changed(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    AttendanceWindow *aw = (AttendanceWindow *)user_data;
    if (aw && aw->date_listbox) {
        printf("Search triggered, loading dates...\n");
        load_attendance_dates(aw);
    }
}

// Clear search callback
static void on_clear_search(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    AttendanceWindow *aw = (AttendanceWindow *)user_data;
    gtk_entry_set_text(GTK_ENTRY(aw->search_entry), "");
    load_attendance_dates(aw);
}

// Delete attendance for selected date
static void on_delete_attendance_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AttendanceWindow *aw = (AttendanceWindow *)user_data;
    
    if (!aw->selected_date || strlen(aw->selected_date) == 0) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please select a date first");
        return;
    }
    
    // Confirmation dialog
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(aw->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Delete all attendance records for %s?",
        aw->selected_date
    );
    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog),
        "This will permanently delete attendance for all students on this date. This action cannot be undone."
    );
    
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Delete from database
        extern sqlite3 *g_db;
        const char *sql = "DELETE FROM attendance WHERE date = ?";
        sqlite3_stmt *stmt;
        
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, aw->selected_date, -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                int deleted = sqlite3_changes(g_db);
                printf("Deleted %d attendance records for %s\n", deleted, aw->selected_date);
                
                char msg[150];
                snprintf(msg, sizeof(msg), "Successfully deleted attendance for %d students on %s", 
                         deleted, aw->selected_date);
                show_info_dialog(GTK_WINDOW(aw->window), msg);
                
                // Clear selected date
                aw->selected_date[0] = '\0';
                
                // Refresh the list
                load_attendance_dates(aw);
                
                // Clear the attendance grid
                GList *children = gtk_container_get_children(GTK_CONTAINER(aw->attendance_grid));
                for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
                    gtk_widget_destroy(GTK_WIDGET(iter->data));
                }
                g_list_free(children);
                
                gtk_label_set_text(GTK_LABEL(aw->stats_label), 
                    "Attendance deleted. Select a date to view.");
            } else {
                show_error_dialog(GTK_WINDOW(aw->window), "Failed to delete attendance");
            }
            sqlite3_finalize(stmt);
        } else {
            show_error_dialog(GTK_WINDOW(aw->window), "Database error");
        }
    }
}

// Refresh mark attendance student list
static void refresh_mark_attendance_list(AttendanceWindow *aw) {
    // Clear existing grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(aw->mark_attendance_grid));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Add headers
    GtkWidget *h1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h1), "<b>Student</b>");
    gtk_widget_set_halign(h1, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), h1, 0, 0, 1, 1);
    
    GtkWidget *h2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h2), "<b>Roll Number</b>");
    gtk_widget_set_halign(h2, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), h2, 1, 0, 1, 1);
    
    GtkWidget *h3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h3), "<b>Status</b>");
    gtk_widget_set_halign(h3, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), h3, 2, 0, 1, 1);
    
    GtkWidget *h4 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(h4), "<b>Notes (Optional)</b>");
    gtk_widget_set_halign(h4, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), h4, 3, 0, 1, 1);
    
    // Get all students for current class
    int count;
    Student **students = db_get_students_by_class(g_session.current_class_id, &count);
    
    if (students) {
        for (int i = 0; i < count; i++) {
            // Student name
            GtkWidget *name_label = gtk_label_new(students[i]->name);
            gtk_widget_set_halign(name_label, GTK_ALIGN_START);
            gtk_widget_set_margin_start(name_label, 10);
            gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), name_label, 0, i + 1, 1, 1);
            
            // Roll number
            GtkWidget *roll_label = gtk_label_new(students[i]->roll_number);
            gtk_widget_set_halign(roll_label, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), roll_label, 1, i + 1, 1, 1);
            
            // Status combo box
            GtkWidget *status_combo = gtk_combo_box_text_new();
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Present");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Absent");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Late");
            gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
            
            // Store student ID in combo box
            g_object_set_data(G_OBJECT(status_combo), "student_id", 
                             GINT_TO_POINTER(students[i]->id));
            
            gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), status_combo, 2, i + 1, 1, 1);
            
            // Notes entry
            GtkWidget *notes_entry = gtk_entry_new();
            gtk_entry_set_placeholder_text(GTK_ENTRY(notes_entry), "Add note...");
            gtk_widget_set_size_request(notes_entry, 200, -1);
            
            // Store student ID in notes entry too
            g_object_set_data(G_OBJECT(notes_entry), "student_id", 
                             GINT_TO_POINTER(students[i]->id));
            
            gtk_grid_attach(GTK_GRID(aw->mark_attendance_grid), notes_entry, 3, i + 1, 1, 1);
        }
        db_free_students(students, count);
    }
    
    // Update stats label
    char stats_text[200];
    snprintf(stats_text, sizeof(stats_text), 
             "Total Students: %d | Ready to mark attendance", count);
    gtk_label_set_text(GTK_LABEL(aw->mark_stats_label), stats_text);
    
    gtk_widget_show_all(aw->mark_attendance_grid);
}

// Mark attendance button callback
static void on_mark_attendance_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AttendanceWindow *aw = (AttendanceWindow *)user_data;
    
    // Get selected date
    const char *date_text = gtk_entry_get_text(GTK_ENTRY(aw->mark_date_entry));
    if (!date_text || strlen(date_text) == 0) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please select a date");
        return;
    }
    
    // Validate date format
    if (!validate_date(date_text)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Invalid date format. Use YYYY-MM-DD");
        return;
    }
    
    // Convert date string to time_t (use noon UTC to avoid timezone issues)
    struct tm tm = {0};
    if (sscanf(date_text, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) != 3) {
        show_error_dialog(GTK_WINDOW(aw->window), "Invalid date format");
        return;
    }
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    tm.tm_hour = 12;  // Set to noon to avoid timezone date shift
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;  // Let mktime determine DST
    time_t date = mktime(&tm);
    
    printf("Date string: %s -> timestamp: %lld\n", date_text, (long long)date);
    
    // Get all combo boxes and save attendance
    GList *children = gtk_container_get_children(GTK_CONTAINER(aw->mark_attendance_grid));
    int saved = 0;
    
    // Create a map to store notes by student_id
    GHashTable *notes_map = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
    
    // First pass: collect all notes
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *widget = GTK_WIDGET(iter->data);
        
        if (GTK_IS_ENTRY(widget)) {
            int student_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "student_id"));
            const char *note_text = gtk_entry_get_text(GTK_ENTRY(widget));
            if (student_id > 0 && note_text && strlen(note_text) > 0) {
                g_hash_table_insert(notes_map, GINT_TO_POINTER(student_id), g_strdup(note_text));
            }
        }
    }
    
    // Second pass: save attendance with notes
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *widget = GTK_WIDGET(iter->data);
        
        // Skip if not a combo box
        if (!GTK_IS_COMBO_BOX(widget)) {
            continue;
        }
        
        int student_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "student_id"));
        if (student_id <= 0) {
            continue;  // Skip widgets without student ID
        }
        
        const char *status_text = gtk_combo_box_text_get_active_text(
            GTK_COMBO_BOX_TEXT(widget));
        
        if (status_text) {
                AttendanceStatus status;
                if (strcmp(status_text, "Present") == 0) {
                    status = ATTENDANCE_PRESENT;
                } else if (strcmp(status_text, "Late") == 0) {
                    status = ATTENDANCE_LATE;
                } else {
                    status = ATTENDANCE_ABSENT;
                }
                
                // Get notes for this student
                const char *notes = g_hash_table_lookup(notes_map, GINT_TO_POINTER(student_id));
                
                printf("Saving attendance: student_id=%d, date=%lld, status=%d, notes=%s\n", 
                       student_id, (long long)date, (int)status, notes ? notes : "(none)");
                if (db_mark_attendance(student_id, date, status, notes ? notes : "")) {
                    saved++;
                    printf("Successfully saved for student %d\n", student_id);
                } else {
                    printf("Failed to save for student %d\n", student_id);
                }
        }
    }
    g_list_free(children);
    g_hash_table_destroy(notes_map);
    
    if (saved > 0) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Attendance saved for %d students on %s", 
                 saved, date_text);
        show_info_dialog(GTK_WINDOW(aw->window), msg);
        
        // Refresh the view tab to show new attendance
        load_attendance_dates(aw);
        
        // Update stats label in Mark tab
        char stats_text[200];
        snprintf(stats_text, sizeof(stats_text), 
                 "Attendance successfully saved for %d students on %s", saved, date_text);
        gtk_label_set_text(GTK_LABEL(aw->mark_stats_label), stats_text);
        
        // Switch to View tab to show the saved attendance
        gtk_notebook_set_current_page(GTK_NOTEBOOK(aw->notebook), 0);
    } else {
        show_error_dialog(GTK_WINDOW(aw->window), "Failed to save attendance");
    }
}

// Create View Attendance tab
static GtkWidget* create_view_tab(AttendanceWindow *aw) {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Header (no frame to avoid white border)
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 15);
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_ctx, "attendance-header");
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='large'>View Attendance Records</span>");
    gtk_box_pack_start(GTK_BOX(header_box), header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);
    
    // Date search section
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkStyleContext *search_box_ctx = gtk_widget_get_style_context(search_box);
    gtk_style_context_add_class(search_box_ctx, "date-search-box");
    
    GtkWidget *search_label = gtk_label_new("Search by Date:");
    gtk_box_pack_start(GTK_BOX(search_box), search_label, FALSE, FALSE, 0);
    
    aw->search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(aw->search_entry), "YYYY-MM-DD");
    gtk_widget_set_size_request(aw->search_entry, 200, -1);
    GtkStyleContext *search_ctx = gtk_widget_get_style_context(aw->search_entry);
    gtk_style_context_add_class(search_ctx, "date-entry");
    gtk_box_pack_start(GTK_BOX(search_box), aw->search_entry, FALSE, FALSE, 0);
    
    GtkWidget *search_btn = gtk_button_new_with_label("Search");
    gtk_widget_set_size_request(search_btn, 100, -1);
    GtkStyleContext *btn_ctx = gtk_widget_get_style_context(search_btn);
    gtk_style_context_add_class(btn_ctx, "mark-button");
    g_signal_connect(search_btn, "clicked", 
                     G_CALLBACK(on_search_changed), aw);
    gtk_box_pack_start(GTK_BOX(search_box), search_btn, FALSE, FALSE, 0);
    
    GtkWidget *clear_btn = gtk_button_new_with_label("Clear");
    gtk_widget_set_size_request(clear_btn, 100, -1);
    g_signal_connect(clear_btn, "clicked",
                     G_CALLBACK(on_clear_search), aw);
    gtk_box_pack_start(GTK_BOX(search_box), clear_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), search_box, FALSE, FALSE, 0);
    
    // Main content area
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(paned), 300);
    
    // Left: Date list
    GtkWidget *left_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(left_scroll, 300, -1);
    
    aw->date_listbox = gtk_list_box_new();
    g_signal_connect(aw->date_listbox, "row-activated",
                     G_CALLBACK(on_date_row_activated), aw);
    gtk_container_add(GTK_CONTAINER(left_scroll), aw->date_listbox);
    gtk_paned_pack1(GTK_PANED(paned), left_scroll, FALSE, FALSE);
    
    // Right: Attendance sheet
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    aw->stats_label = gtk_label_new("Select a date to view attendance");
    GtkStyleContext *stats_ctx = gtk_widget_get_style_context(aw->stats_label);
    gtk_style_context_add_class(stats_ctx, "stats-label");
    gtk_box_pack_start(GTK_BOX(right_box), aw->stats_label, FALSE, FALSE, 0);
    
    // Delete button
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete Date");
    gtk_widget_set_size_request(delete_btn, 100, -1);
    GtkStyleContext *del_ctx = gtk_widget_get_style_context(delete_btn);
    gtk_style_context_add_class(del_ctx, "danger-button");
    g_signal_connect(delete_btn, "clicked",
                     G_CALLBACK(on_delete_attendance_clicked), aw);
    gtk_box_pack_start(GTK_BOX(right_box), delete_btn, FALSE, FALSE, 5);
    
    GtkWidget *right_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(right_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    aw->attendance_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(aw->attendance_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(aw->attendance_grid), 20);
    gtk_widget_set_margin_start(aw->attendance_grid, 20);
    gtk_widget_set_margin_end(aw->attendance_grid, 20);
    gtk_widget_set_margin_top(aw->attendance_grid, 10);
    gtk_widget_set_margin_bottom(aw->attendance_grid, 10);
    
    GtkStyleContext *grid_ctx = gtk_widget_get_style_context(aw->attendance_grid);
    gtk_style_context_add_class(grid_ctx, "attendance-section");
    
    gtk_container_add(GTK_CONTAINER(right_scroll), aw->attendance_grid);
    gtk_box_pack_start(GTK_BOX(right_box), right_scroll, TRUE, TRUE, 0);
    
    gtk_paned_pack2(GTK_PANED(paned), right_box, TRUE, FALSE);
    gtk_box_pack_start(GTK_BOX(main_box), paned, TRUE, TRUE, 0);
    
    return main_box;
}

// Create Mark Attendance tab
static GtkWidget* create_mark_tab(AttendanceWindow *aw) {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 20);
    gtk_widget_set_margin_end(main_box, 20);
    gtk_widget_set_margin_top(main_box, 20);
    gtk_widget_set_margin_bottom(main_box, 20);
    
    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='large'>Mark Student Attendance</span>");
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header);
    gtk_style_context_add_class(header_ctx, "attendance-header");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    
    // Date selection with styled container
    GtkWidget *date_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkStyleContext *container_ctx = gtk_widget_get_style_context(date_container);
    gtk_style_context_add_class(container_ctx, "date-search-box");
    
    GtkWidget *date_label = gtk_label_new("Select Date:");
    gtk_box_pack_start(GTK_BOX(date_container), date_label, FALSE, FALSE, 0);
    
    aw->mark_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(aw->mark_date_entry), "YYYY-MM-DD");
    gtk_widget_set_size_request(aw->mark_date_entry, 200, -1);
    GtkStyleContext *entry_ctx = gtk_widget_get_style_context(aw->mark_date_entry);
    gtk_style_context_add_class(entry_ctx, "date-entry");
    
    // Set today's date as default
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char today[20];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_info);
    gtk_entry_set_text(GTK_ENTRY(aw->mark_date_entry), today);
    
    gtk_box_pack_start(GTK_BOX(date_container), aw->mark_date_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), date_container, FALSE, FALSE, 0);
    
    // Student list scroll
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    aw->mark_attendance_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(aw->mark_attendance_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(aw->mark_attendance_grid), 20);
    gtk_widget_set_margin_start(aw->mark_attendance_grid, 10);
    gtk_widget_set_margin_end(aw->mark_attendance_grid, 10);
    gtk_widget_set_margin_top(aw->mark_attendance_grid, 10);
    gtk_widget_set_margin_bottom(aw->mark_attendance_grid, 10);
    
    GtkStyleContext *grid_ctx = gtk_widget_get_style_context(aw->mark_attendance_grid);
    gtk_style_context_add_class(grid_ctx, "attendance-section");
    
    gtk_container_add(GTK_CONTAINER(scroll), aw->mark_attendance_grid);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
    
    // Stats label for Mark tab
    aw->mark_stats_label = gtk_label_new("Total Students: 0 | Present: 0 | Absent: 0 | Late: 0");
    GtkStyleContext *mark_stats_ctx = gtk_widget_get_style_context(aw->mark_stats_label);
    gtk_style_context_add_class(mark_stats_ctx, "stats-label");
    gtk_box_pack_start(GTK_BOX(main_box), aw->mark_stats_label, FALSE, FALSE, 5);
    
    // Button container
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkStyleContext *btn_box_ctx = gtk_widget_get_style_context(button_box);
    gtk_style_context_add_class(btn_box_ctx, "button-container");
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    
    // Save button
    GtkWidget *save_btn = gtk_button_new_with_label("Save Attendance");
    gtk_widget_set_size_request(save_btn, 180, 40);
    GtkStyleContext *save_ctx = gtk_widget_get_style_context(save_btn);
    gtk_style_context_add_class(save_ctx, "mark-button");
    g_signal_connect(save_btn, "clicked", 
                     G_CALLBACK(on_mark_attendance_clicked), aw);
    gtk_box_pack_start(GTK_BOX(button_box), save_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 10);
    
    return main_box;
}

// Main show attendance window function
void show_attendance_window(GtkWidget *parent) {
    AttendanceWindow *aw = g_new0(AttendanceWindow, 1);
    aw->selected_date[0] = '\0';
    
    // Apply CSS styling
    apply_attendance_css();
    
    // Create window
    aw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(aw->window), "Attendance Management");
    gtk_window_set_default_size(GTK_WINDOW(aw->window), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(aw->window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(aw->window), GTK_WINDOW(parent));
    
    // Create notebook with tabs
    aw->notebook = gtk_notebook_new();
    
    // Add View tab
    GtkWidget *view_tab = create_view_tab(aw);
    gtk_notebook_append_page(GTK_NOTEBOOK(aw->notebook), view_tab,
                            gtk_label_new("View Attendance"));
    
    // Add Mark tab
    GtkWidget *mark_tab = create_mark_tab(aw);
    gtk_notebook_append_page(GTK_NOTEBOOK(aw->notebook), mark_tab,
                            gtk_label_new("Mark Attendance"));
    
    gtk_container_add(GTK_CONTAINER(aw->window), aw->notebook);
    
    // Load initial data
    load_attendance_dates(aw);
    refresh_mark_attendance_list(aw);
    
    // Cleanup on close
    g_signal_connect_swapped(aw->window, "destroy",
                             G_CALLBACK(g_free), aw);
    
    gtk_widget_show_all(aw->window);
}
