/**
 * UI Reports Module - Analytics and Statistics
 * Provides comprehensive reports for attendance, assignments, and student performance
 */

#include <gtk/gtk.h>
#include <time.h>
#include <string.h>
#include "database.h"
#include "common.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *attendance_tree;
    GtkWidget *assignment_tree;
    GtkWidget *student_combo;
    GtkWidget *student_report_box;
} ReportsWindow;

// Forward declarations
static void create_attendance_report_tab(ReportsWindow *rw);
static void create_assignment_report_tab(ReportsWindow *rw);
static void create_student_report_tab(ReportsWindow *rw);
static void refresh_attendance_report(ReportsWindow *rw);
static void refresh_assignment_report(ReportsWindow *rw);
static void on_student_selected(GtkComboBox *combo, gpointer user_data);

// CSS Styling
static void apply_reports_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        ".report-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 15px;"
        "  border-radius: 8px;"
        "  margin: 10px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        ".report-card {"
        "  background: white;"
        "  border-radius: 8px;"
        "  padding: 15px;"
        "  margin: 10px;"
        "  box-shadow: 0 2px 8px rgba(0,0,0,0.1);"
        "}"
        ".stat-box {"
        "  background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);"
        "  color: white;"
        "  padding: 15px;"
        "  border-radius: 6px;"
        "  margin: 5px;"
        "}"
        ".refresh-button {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 8px 20px;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

// Create Attendance Summary Report Tab
static void create_attendance_report_tab(ReportsWindow *rw) {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    
    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='large'>Student Attendance Summary</span>");
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header);
    gtk_style_context_add_class(header_ctx, "report-header");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    
    // Refresh button
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh Report");
    gtk_widget_set_size_request(refresh_btn, 150, -1);
    GtkStyleContext *btn_ctx = gtk_widget_get_style_context(refresh_btn);
    gtk_style_context_add_class(btn_ctx, "refresh-button");
    g_signal_connect_swapped(refresh_btn, "clicked",
                             G_CALLBACK(refresh_attendance_report), rw);
    gtk_box_pack_start(GTK_BOX(main_box), refresh_btn, FALSE, FALSE, 0);
    
    // Tree view with scrolled window
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Create list store: Student Name, Roll No, Total Days, Present, Absent, Late, Attendance %
    GtkListStore *store = gtk_list_store_new(7, 
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, 
        G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
    
    rw->attendance_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Add columns
    const char *headers[] = {"Student Name", "Roll Number", "Total Days", 
                            "Present", "Absent", "Late", "Attendance %"};
    for (int i = 0; i < 7; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            headers[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, i);
        gtk_tree_view_append_column(GTK_TREE_VIEW(rw->attendance_tree), column);
    }
    
    gtk_container_add(GTK_CONTAINER(scroll), rw->attendance_tree);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(rw->notebook), main_box,
                            gtk_label_new("Attendance Summary"));
}

// Create Assignment Statistics Report Tab
static void create_assignment_report_tab(ReportsWindow *rw) {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    
    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='large'>Assignment Completion Statistics</span>");
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header);
    gtk_style_context_add_class(header_ctx, "report-header");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    
    // Refresh button
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh Report");
    gtk_widget_set_size_request(refresh_btn, 150, -1);
    GtkStyleContext *btn_ctx = gtk_widget_get_style_context(refresh_btn);
    gtk_style_context_add_class(btn_ctx, "refresh-button");
    g_signal_connect_swapped(refresh_btn, "clicked",
                             G_CALLBACK(refresh_assignment_report), rw);
    gtk_box_pack_start(GTK_BOX(main_box), refresh_btn, FALSE, FALSE, 0);
    
    // Tree view
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Store: Assignment Title, Subject, Total Students, Completed, Pending, Completion %
    GtkListStore *store = gtk_list_store_new(6, 
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, 
        G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
    
    rw->assignment_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    const char *headers[] = {"Assignment", "Subject", "Total Students", 
                            "Completed", "Pending", "Completion %"};
    for (int i = 0; i < 6; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            headers[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, i);
        gtk_tree_view_append_column(GTK_TREE_VIEW(rw->assignment_tree), column);
    }
    
    gtk_container_add(GTK_CONTAINER(scroll), rw->assignment_tree);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(rw->notebook), main_box,
                            gtk_label_new("Assignment Statistics"));
}

// Create Individual Student Report Tab
static void create_student_report_tab(ReportsWindow *rw) {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    
    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='large'>Individual Student Report</span>");
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header);
    gtk_style_context_add_class(header_ctx, "report-header");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    
    // Student selection
    GtkWidget *select_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *select_label = gtk_label_new("Select Student:");
    gtk_box_pack_start(GTK_BOX(select_box), select_label, FALSE, FALSE, 0);
    
    rw->student_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(rw->student_combo, 300, -1);
    
    // Populate with students from current class
    int count;
    Student **students = db_get_students_by_class(g_session.current_class_id, &count);
    if (students) {
        for (int i = 0; i < count; i++) {
            char display[200];
            snprintf(display, sizeof(display), "%s - %s", 
                     students[i]->roll_number, students[i]->name);
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rw->student_combo), display);
            g_object_set_data(G_OBJECT(rw->student_combo), 
                            g_strdup_printf("student_id_%d", i), 
                            GINT_TO_POINTER(students[i]->id));
        }
        db_free_students(students, count);
    }
    
    g_signal_connect(rw->student_combo, "changed",
                     G_CALLBACK(on_student_selected), rw);
    gtk_box_pack_start(GTK_BOX(select_box), rw->student_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), select_box, FALSE, FALSE, 10);
    
    // Report display area
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    rw->student_report_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(rw->student_report_box, 10);
    gtk_widget_set_margin_end(rw->student_report_box, 10);
    
    GtkWidget *placeholder = gtk_label_new("Select a student to view their detailed report");
    gtk_box_pack_start(GTK_BOX(rw->student_report_box), placeholder, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(scroll), rw->student_report_box);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(rw->notebook), main_box,
                            gtk_label_new("Student Report"));
}

// Refresh Attendance Report
static void refresh_attendance_report(ReportsWindow *rw) {
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(rw->attendance_tree));
    gtk_list_store_clear(GTK_LIST_STORE(model));
    
    extern sqlite3 *g_db;
    if (!g_db) return;
    
    // Get all students with their attendance statistics
    const char *sql = 
        "SELECT s.name, s.roll_number, "
        "COUNT(DISTINCT a.date) as total_days, "
        "SUM(CASE WHEN a.status = 1 THEN 1 ELSE 0 END) as present, "
        "SUM(CASE WHEN a.status = 0 THEN 1 ELSE 0 END) as absent, "
        "SUM(CASE WHEN a.status = 2 THEN 1 ELSE 0 END) as late "
        "FROM students s "
        "LEFT JOIN attendance a ON s.id = a.student_id "
        "WHERE s.class_id = ? "
        "GROUP BY s.id, s.name, s.roll_number "
        "ORDER BY s.roll_number";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, g_session.current_class_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            const char *roll = (const char *)sqlite3_column_text(stmt, 1);
            int total = sqlite3_column_int(stmt, 2);
            int present = sqlite3_column_int(stmt, 3);
            int absent = sqlite3_column_int(stmt, 4);
            int late = sqlite3_column_int(stmt, 5);
            
            char percentage[20];
            if (total > 0) {
                float pct = (float)(present + late) / total * 100;
                snprintf(percentage, sizeof(percentage), "%.1f%%", pct);
            } else {
                strcpy(percentage, "N/A");
            }
            
            GtkTreeIter iter;
            gtk_list_store_append(GTK_LIST_STORE(model), &iter);
            gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                             0, name,
                             1, roll,
                             2, total,
                             3, present,
                             4, absent,
                             5, late,
                             6, percentage,
                             -1);
        }
        sqlite3_finalize(stmt);
    }
}

// Refresh Assignment Report
static void refresh_assignment_report(ReportsWindow *rw) {
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(rw->assignment_tree));
    gtk_list_store_clear(GTK_LIST_STORE(model));
    
    extern sqlite3 *g_db;
    if (!g_db) return;
    
    // Get total students count
    const char *count_sql = "SELECT COUNT(*) FROM students WHERE class_id = ?";
    sqlite3_stmt *count_stmt;
    int total_students = 0;
    
    if (sqlite3_prepare_v2(g_db, count_sql, -1, &count_stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(count_stmt, 1, g_session.current_class_id);
        if (sqlite3_step(count_stmt) == SQLITE_ROW) {
            total_students = sqlite3_column_int(count_stmt, 0);
        }
        sqlite3_finalize(count_stmt);
    }
    
    // Get assignment statistics
    const char *sql = 
        "SELECT a.title, a.subject, "
        "COUNT(CASE WHEN s.status IN (1, 2) THEN 1 END) as completed "
        "FROM assignments a "
        "LEFT JOIN assignment_submissions s ON a.id = s.assignment_id "
        "WHERE a.class_id = ? "
        "GROUP BY a.id, a.title, a.subject "
        "ORDER BY a.title";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, g_session.current_class_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *title = (const char *)sqlite3_column_text(stmt, 0);
            const char *subject = (const char *)sqlite3_column_text(stmt, 1);
            int completed = sqlite3_column_int(stmt, 2);
            int pending = total_students - completed;
            
            char percentage[20];
            if (total_students > 0) {
                float pct = (float)completed / total_students * 100;
                snprintf(percentage, sizeof(percentage), "%.1f%%", pct);
            } else {
                strcpy(percentage, "N/A");
            }
            
            GtkTreeIter iter;
            gtk_list_store_append(GTK_LIST_STORE(model), &iter);
            gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                             0, title,
                             1, subject,
                             2, total_students,
                             3, completed,
                             4, pending,
                             5, percentage,
                             -1);
        }
        sqlite3_finalize(stmt);
    }
}

// Student Selected Callback
static void on_student_selected(GtkComboBox *combo, gpointer user_data) {
    ReportsWindow *rw = (ReportsWindow *)user_data;
    
    // Clear previous report
    GList *children = gtk_container_get_children(GTK_CONTAINER(rw->student_report_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if (index < 0) return;
    
    char key[50];
    snprintf(key, sizeof(key), "student_id_%d", index);
    int student_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(combo), key));
    
    if (student_id <= 0) return;
    
    extern sqlite3 *g_db;
    
    // Get student info
    Student *student = db_get_student_by_id(student_id);
    if (!student) return;
    
    // Student info card
    GtkWidget *info_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkStyleContext *card_ctx = gtk_widget_get_style_context(info_card);
    gtk_style_context_add_class(card_ctx, "report-card");
    
    char info_text[500];
    snprintf(info_text, sizeof(info_text),
             "<b>Student Information</b>\n"
             "Name: %s\n"
             "Roll Number: %s\n"
             "Email: %s\n"
             "Phone: %s",
             student->name, student->roll_number, 
             student->email, student->phone);
    
    GtkWidget *info_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(info_label), info_text);
    gtk_widget_set_halign(info_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(info_card), info_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(rw->student_report_box), info_card, FALSE, FALSE, 5);
    
    free(student);
    
    // Attendance statistics
    const char *att_sql = 
        "SELECT "
        "COUNT(DISTINCT date) as total, "
        "SUM(CASE WHEN status = 1 THEN 1 ELSE 0 END) as present, "
        "SUM(CASE WHEN status = 0 THEN 1 ELSE 0 END) as absent, "
        "SUM(CASE WHEN status = 2 THEN 1 ELSE 0 END) as late "
        "FROM attendance WHERE student_id = ?";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(g_db, att_sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int total = sqlite3_column_int(stmt, 0);
            int present = sqlite3_column_int(stmt, 1);
            int absent = sqlite3_column_int(stmt, 2);
            int late = sqlite3_column_int(stmt, 3);
            
            GtkWidget *att_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
            GtkStyleContext *att_ctx = gtk_widget_get_style_context(att_card);
            gtk_style_context_add_class(att_ctx, "report-card");
            
            char att_text[300];
            float pct = total > 0 ? (float)(present + late) / total * 100 : 0;
            snprintf(att_text, sizeof(att_text),
                     "<b>Attendance Summary</b>\n"
                     "Total Days: %d | Present: %d | Absent: %d | Late: %d\n"
                     "Attendance Percentage: %.1f%%",
                     total, present, absent, late, pct);
            
            GtkWidget *att_label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(att_label), att_text);
            gtk_widget_set_halign(att_label, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(att_card), att_label, FALSE, FALSE, 5);
            gtk_box_pack_start(GTK_BOX(rw->student_report_box), att_card, FALSE, FALSE, 5);
        }
        sqlite3_finalize(stmt);
    }
    
    // Assignment statistics
    const char *assign_sql = 
        "SELECT "
        "COUNT(*) as total, "
        "SUM(CASE WHEN s.status IN (1, 2) THEN 1 ELSE 0 END) as completed "
        "FROM assignments a "
        "LEFT JOIN assignment_submissions s ON a.id = s.assignment_id AND s.student_id = ? "
        "WHERE a.class_id = ?";
    
    if (sqlite3_prepare_v2(g_db, assign_sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        sqlite3_bind_int(stmt, 2, g_session.current_class_id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int total = sqlite3_column_int(stmt, 0);
            int completed = sqlite3_column_int(stmt, 1);
            
            GtkWidget *assign_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
            GtkStyleContext *assign_ctx = gtk_widget_get_style_context(assign_card);
            gtk_style_context_add_class(assign_ctx, "report-card");
            
            char assign_text[300];
            float pct = total > 0 ? (float)completed / total * 100 : 0;
            snprintf(assign_text, sizeof(assign_text),
                     "<b>Assignment Completion</b>\n"
                     "Total Assignments: %d | Completed: %d | Pending: %d\n"
                     "Completion Rate: %.1f%%",
                     total, completed, total - completed, pct);
            
            GtkWidget *assign_label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(assign_label), assign_text);
            gtk_widget_set_halign(assign_label, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(assign_card), assign_label, FALSE, FALSE, 5);
            gtk_box_pack_start(GTK_BOX(rw->student_report_box), assign_card, FALSE, FALSE, 5);
        }
        sqlite3_finalize(stmt);
    }
    
    gtk_widget_show_all(rw->student_report_box);
}

// Main show reports window function
void show_reports_window(GtkWindow *parent) {
    ReportsWindow *rw = g_new0(ReportsWindow, 1);
    
    // Apply CSS
    apply_reports_css();
    
    // Create window
    rw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(rw->window), "Reports & Analytics");
    gtk_window_set_default_size(GTK_WINDOW(rw->window), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(rw->window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(rw->window), parent);
    
    // Create notebook
    rw->notebook = gtk_notebook_new();
    
    // Create tabs
    create_attendance_report_tab(rw);
    create_assignment_report_tab(rw);
    create_student_report_tab(rw);
    
    gtk_container_add(GTK_CONTAINER(rw->window), rw->notebook);
    
    // Load initial data
    refresh_attendance_report(rw);
    refresh_assignment_report(rw);
    
    // Cleanup on close
    g_signal_connect_swapped(rw->window, "destroy",
                             G_CALLBACK(g_free), rw);
    
    gtk_widget_show_all(rw->window);
}
