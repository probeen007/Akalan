#include "ui_dashboard.h"
#include "ui_students.h"
#include "ui_assignments.h"
#include "ui_attendance.h"
#include "ui_reports.h"
#include "ui_classes.h"
#include "database.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *class_combo;
    GtkWidget *students_button;
    GtkWidget *assignments_button;
    GtkWidget *attendance_button;
    GtkWidget *reports_button;
} DashboardWindow;

// Global pointer for callback (not ideal but works for single dashboard)
static DashboardWindow *g_dashboard = NULL;

// Apply CSS styling for dashboard
static void apply_dashboard_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        ".dashboard-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 20px;"
        "  border-radius: 10px;"
        "  margin: 5px;"
        "}"
        ".module-card {"
        "  background: white;"
        "  border: 2px solid #e0e0e0;"
        "  border-radius: 12px;"
        "  padding: 20px;"
        "  transition: all 0.3s ease;"
        "}"
        ".module-card:hover {"
        "  border-color: #667eea;"
        "  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.3);"
        "}"
        ".module-title {"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  color: #333;"
        "}"
        ".module-desc {"
        "  font-size: 12px;"
        "  color: #666;"
        "}"
        ".logout-btn {"
        "  background: #dc3545;"
        "  color: white;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "}"
        ".class-selector {"
        "  background: white;"
        "  border: 2px solid #667eea;"
        "  border-radius: 8px;"
        "  padding: 15px;"
        "  margin: 10px 0;"
        "}"
        ".create-class-card {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  border: 2px solid #e0e0e0;"
        "  border-radius: 12px;"
        "  padding: 20px;"
        "  transition: all 0.3s ease;"
        "}"
        ".create-class-card:hover {"
        "  border-color: #667eea;"
        "  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.3);"
        "}";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static void update_button_states(DashboardWindow *dw) {
    gboolean has_class = (g_session.current_class_id > 0);
    
    gtk_widget_set_sensitive(dw->students_button, has_class);
    gtk_widget_set_sensitive(dw->assignments_button, has_class);
    gtk_widget_set_sensitive(dw->attendance_button, has_class);
    gtk_widget_set_sensitive(dw->reports_button, has_class);
}

static void on_class_changed(GtkComboBox *combo, gpointer user_data) {
    DashboardWindow *dw = (DashboardWindow *)user_data;
    
    GtkTreeIter iter;
    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        GtkTreeModel *model = gtk_combo_box_get_model(combo);
        gint class_id;
        gtk_tree_model_get(model, &iter, 0, &class_id, -1);
        
        g_session.current_class_id = class_id;
        update_button_states(dw);
    } else {
        g_session.current_class_id = 0;
        update_button_states(dw);
    }
}

static void refresh_class_dropdown(DashboardWindow *dw) {
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
    
    int count = 0;
    Class **classes = db_get_user_classes(g_session.user_id, &count);
    
    if (classes && count > 0) {
        for (int i = 0; i < count; i++) {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                             0, classes[i]->id,
                             1, classes[i]->name,
                             -1);
        }
        db_free_classes(classes, count);
    }
    
    gtk_combo_box_set_model(GTK_COMBO_BOX(dw->class_combo), GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Don't auto-select - user must choose a class
    gtk_combo_box_set_active(GTK_COMBO_BOX(dw->class_combo), -1);
}

static void refresh_class_dropdown_callback(void) {
    if (g_dashboard) {
        refresh_class_dropdown(g_dashboard);
    }
}

static void on_create_class_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DashboardWindow *dw = (DashboardWindow *)user_data;
    
    show_create_class_window(GTK_WINDOW(dw->window), 
        refresh_class_dropdown_callback);
}

static void on_delete_class_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DashboardWindow *dw = (DashboardWindow *)user_data;
    
    if (g_session.current_class_id == 0) {
        show_error_dialog(GTK_WINDOW(dw->window), "Please select a class to delete.");
        return;
    }
    
    // Get class name and counts for confirmation
    Class *class_info = db_get_class_by_id(g_session.current_class_id);
    if (!class_info) {
        show_error_dialog(GTK_WINDOW(dw->window), "Failed to get class information.");
        return;
    }
    
    // Get actual counts of data to be deleted
    int student_count = 0;
    int assignment_count = 0;
    Student **students = db_get_students_by_class(g_session.current_class_id, &student_count);
    Assignment **assignments = db_get_assignments_by_class(g_session.current_class_id, &assignment_count);
    
    db_free_students(students, student_count);
    db_free_assignments(assignments, assignment_count);
    
    char confirm_msg[512];
    snprintf(confirm_msg, sizeof(confirm_msg),
        "Are you sure you want to delete '%s'?\n\n"
        "This will permanently delete:\n"
        "• %d student%s\n"
        "• %d assignment%s\n"
        "• All related attendance and submission records\n\n"
        "This action cannot be undone!",
        class_info->name,
        student_count, student_count == 1 ? "" : "s",
        assignment_count, assignment_count == 1 ? "" : "s");
    
    bool confirmed = show_confirm_dialog(GTK_WINDOW(dw->window), confirm_msg);
    
    if (confirmed) {
        if (db_delete_class(g_session.current_class_id)) {
            show_info_dialog(GTK_WINDOW(dw->window), "Class deleted successfully!");
            g_session.current_class_id = 0;
            refresh_class_dropdown(dw);
            update_button_states(dw);
        } else {
            show_error_dialog(GTK_WINDOW(dw->window), "Failed to delete class.");
        }
    }
    
    free(class_info);
}

static void check_class_selected(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    if (g_session.current_class_id == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "Please Select a Class");
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
            "You must select a class before accessing this feature.\n\n"
            "Please select a class from the dropdown above or create a new class.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void on_students_clicked(GtkButton *button, gpointer user_data) {
    DashboardWindow *dw = (DashboardWindow *)user_data;
    if (g_session.current_class_id == 0) {
        check_class_selected(button, user_data);
        return;
    }
    show_students_window(GTK_WINDOW(dw->window));
}

static void on_assignments_clicked(GtkButton *button, gpointer user_data) {
    DashboardWindow *dw = (DashboardWindow *)user_data;
    if (g_session.current_class_id == 0) {
        check_class_selected(button, user_data);
        return;
    }
    show_assignments_window(GTK_WINDOW(dw->window));
}

static void on_attendance_clicked(GtkButton *button, gpointer user_data) {
    DashboardWindow *dw = (DashboardWindow *)user_data;
    if (g_session.current_class_id == 0) {
        check_class_selected(button, user_data);
        return;
    }
    show_attendance_window(GTK_WINDOW(dw->window));
}

static void on_reports_clicked(GtkButton *button, gpointer user_data) {
    DashboardWindow *dw = (DashboardWindow *)user_data;
    if (g_session.current_class_id == 0) {
        check_class_selected(button, user_data);
        return;
    }
    show_reports_window(GTK_WINDOW(dw->window));
}

static void on_logout_clicked(GtkButton *button, gpointer user_data) {
    DashboardWindow *dw = (DashboardWindow *)user_data;
    
    if (show_confirm_dialog(GTK_WINDOW(dw->window), "Are you sure you want to logout?")) {
        g_session.is_logged_in = false;
        g_session.user_id = 0;
        g_session.current_class_id = 0;
        gtk_widget_destroy(dw->window);
        gtk_main_quit();
    }
}

void show_dashboard_window(void) {
    DashboardWindow *dw = g_malloc(sizeof(DashboardWindow));
    g_dashboard = dw;  // Set global for callbacks
    
    // Initialize session
    g_session.current_class_id = 0;
    
    // Apply CSS
    apply_dashboard_css();
    
    // Create window
    dw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    char title[256];
    snprintf(title, sizeof(title), "Aakalan - Welcome %s", g_session.name);
    gtk_window_set_title(GTK_WINDOW(dw->window), title);
    gtk_window_set_default_size(GTK_WINDOW(dw->window), 900, 750);
    gtk_window_set_position(GTK_WINDOW(dw->window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(dw->window), 15);
    
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    // Header (no frame to avoid white border)
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 20);
    gtk_widget_set_size_request(header_box, -1, 140);
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_ctx, "dashboard-header");
    
    GtkWidget *welcome_label = gtk_label_new(NULL);
    char welcome_text[256];
    snprintf(welcome_text, sizeof(welcome_text), 
             "<span size='xx-large' weight='bold'>Welcome, %s!</span>\n\n"
             "<span size='small'>Select a module below to get started</span>", 
             g_session.name);
    gtk_label_set_markup(GTK_LABEL(welcome_label), welcome_text);
    gtk_box_pack_start(GTK_BOX(header_box), welcome_label, TRUE, TRUE, 0);
    
    GtkWidget *logout_button = gtk_button_new_with_label("Logout");
    gtk_widget_set_size_request(logout_button, 100, 40);
    gtk_widget_set_valign(logout_button, GTK_ALIGN_CENTER);
    GtkStyleContext *logout_ctx = gtk_widget_get_style_context(logout_button);
    gtk_style_context_add_class(logout_ctx, "logout-btn");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(on_logout_clicked), dw);
    gtk_box_pack_end(GTK_BOX(header_box), logout_button, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);
    
    // Class selector section (centered, no border)
    GtkWidget *class_selector_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(class_selector_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(class_selector_box, 10);
    gtk_widget_set_margin_bottom(class_selector_box, 10);
    
    GtkWidget *class_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(class_label), 
        "<span weight='bold' size='large'>Select Class:</span>");
    gtk_box_pack_start(GTK_BOX(class_selector_box), class_label, FALSE, FALSE, 0);
    
    dw->class_combo = gtk_combo_box_new();
    gtk_widget_set_size_request(dw->class_combo, 200, -1);
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(dw->class_combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(dw->class_combo), renderer, "text", 1, NULL);
    g_signal_connect(dw->class_combo, "changed", G_CALLBACK(on_class_changed), dw);
    gtk_box_pack_start(GTK_BOX(class_selector_box), dw->class_combo, FALSE, FALSE, 0);
    
    // Create new class button
    GtkWidget *create_class_btn = gtk_button_new();
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *btn_icon = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(btn_icon), "<span size='large' foreground='white'>➕</span>");
    GtkWidget *btn_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(btn_label), "<span foreground='white'>Create New Class</span>");
    gtk_box_pack_start(GTK_BOX(btn_box), btn_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), btn_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(create_class_btn), btn_box);
    
    // Apply inline CSS for blue button
    GtkCssProvider *btn_provider = gtk_css_provider_new();
    const char *btn_css = 
        "button { "
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); "
        "  border: none; "
        "  border-radius: 6px; "
        "  padding: 8px 16px; "
        "}"
        "button:hover { "
        "  background: linear-gradient(135deg, #5568d3 0%, #653a8b 100%); "
        "}";
    gtk_css_provider_load_from_data(btn_provider, btn_css, -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(create_class_btn),
        GTK_STYLE_PROVIDER(btn_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(btn_provider);
    
    g_signal_connect(create_class_btn, "clicked", G_CALLBACK(on_create_class_clicked), dw);
    gtk_box_pack_start(GTK_BOX(class_selector_box), create_class_btn, FALSE, FALSE, 0);
    
    // Delete class button
    GtkWidget *delete_class_btn = gtk_button_new();
    GtkWidget *del_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *del_icon = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(del_icon), "<span size='large' foreground='white'>🗑️</span>");
    GtkWidget *del_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(del_label), "<span foreground='white'>Delete Class</span>");
    gtk_box_pack_start(GTK_BOX(del_box), del_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(del_box), del_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(delete_class_btn), del_box);
    
    // Apply red styling for delete button
    GtkCssProvider *del_provider = gtk_css_provider_new();
    const char *del_css = 
        "button { "
        "  background: #dc3545; "
        "  border: none; "
        "  border-radius: 6px; "
        "  padding: 8px 16px; "
        "}"
        "button:hover { "
        "  background: #c82333; "
        "}";
    gtk_css_provider_load_from_data(del_provider, del_css, -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(delete_class_btn),
        GTK_STYLE_PROVIDER(del_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(del_provider);
    
    g_signal_connect(delete_class_btn, "clicked", G_CALLBACK(on_delete_class_clicked), dw);
    gtk_box_pack_start(GTK_BOX(class_selector_box), delete_class_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), class_selector_box, FALSE, FALSE, 0);
    
    // Grid for module buttons
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    
    // Create module buttons
    // Students button
    dw->students_button = gtk_button_new();
    GtkStyleContext *s_ctx = gtk_widget_get_style_context(dw->students_button);
    gtk_style_context_add_class(s_ctx, "module-card");
    GtkWidget *students_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(students_box), 15);
    
    GtkWidget *students_icon = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(students_icon), "<span size='xx-large'>🎓</span>");
    gtk_widget_set_halign(students_icon, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(students_box), students_icon, FALSE, FALSE, 0);
    
    GtkWidget *students_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(students_label), 
        "<span size='large' weight='bold'>Student Management</span>");
    GtkWidget *students_desc = gtk_label_new("Add, edit, and manage student records");
    gtk_box_pack_start(GTK_BOX(students_box), students_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(students_box), students_desc, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dw->students_button), students_box);
    gtk_widget_set_size_request(dw->students_button, 280, 120);
    g_signal_connect(dw->students_button, "clicked", G_CALLBACK(on_students_clicked), dw);
    gtk_grid_attach(GTK_GRID(grid), dw->students_button, 0, 0, 1, 1);
    
    // Assignments button
    dw->assignments_button = gtk_button_new();
    GtkStyleContext *a_ctx = gtk_widget_get_style_context(dw->assignments_button);
    gtk_style_context_add_class(a_ctx, "module-card");
    GtkWidget *assignments_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(assignments_box), 15);
    
    GtkWidget *assignments_icon = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(assignments_icon), "<span size='xx-large'>📚</span>");
    gtk_widget_set_halign(assignments_icon, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(assignments_box), assignments_icon, FALSE, FALSE, 0);
    
    GtkWidget *assignments_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(assignments_label), 
        "<span size='large' weight='bold'>Assignment Tracking</span>");
    GtkWidget *assignments_desc = gtk_label_new("Create and track assignment submissions");
    gtk_box_pack_start(GTK_BOX(assignments_box), assignments_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(assignments_box), assignments_desc, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dw->assignments_button), assignments_box);
    gtk_widget_set_size_request(dw->assignments_button, 280, 120);
    g_signal_connect(dw->assignments_button, "clicked", G_CALLBACK(on_assignments_clicked), dw);
    gtk_grid_attach(GTK_GRID(grid), dw->assignments_button, 1, 0, 1, 1);
    
    // Attendance button
    dw->attendance_button = gtk_button_new();
    GtkStyleContext *att_ctx = gtk_widget_get_style_context(dw->attendance_button);
    gtk_style_context_add_class(att_ctx, "module-card");
    GtkWidget *attendance_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(attendance_box), 15);
    
    GtkWidget *attendance_icon = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(attendance_icon), "<span size='xx-large'>✅</span>");
    gtk_widget_set_halign(attendance_icon, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(attendance_box), attendance_icon, FALSE, FALSE, 0);
    
    GtkWidget *attendance_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(attendance_label), 
        "<span size='large' weight='bold'>Attendance Management</span>");
    GtkWidget *attendance_desc = gtk_label_new("Mark and review student attendance");
    gtk_box_pack_start(GTK_BOX(attendance_box), attendance_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(attendance_box), attendance_desc, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dw->attendance_button), attendance_box);
    gtk_widget_set_size_request(dw->attendance_button, 280, 120);
    g_signal_connect(dw->attendance_button, "clicked", G_CALLBACK(on_attendance_clicked), dw);
    gtk_grid_attach(GTK_GRID(grid), dw->attendance_button, 2, 0, 1, 1);
    
    // Reports button
    dw->reports_button = gtk_button_new();
    GtkStyleContext *r_ctx = gtk_widget_get_style_context(dw->reports_button);
    gtk_style_context_add_class(r_ctx, "module-card");
    GtkWidget *reports_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(reports_box), 15);
    
    GtkWidget *reports_icon = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(reports_icon), "<span size='xx-large'>📊</span>");
    gtk_widget_set_halign(reports_icon, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(reports_box), reports_icon, FALSE, FALSE, 0);
    
    GtkWidget *reports_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(reports_label), 
        "<span size='large' weight='bold'>Reports &amp; Analytics</span>");
    GtkWidget *reports_desc = gtk_label_new("View statistics and generate reports");
    gtk_box_pack_start(GTK_BOX(reports_box), reports_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(reports_box), reports_desc, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dw->reports_button), reports_box);
    gtk_widget_set_size_request(dw->reports_button, 280, 120);
    g_signal_connect(dw->reports_button, "clicked", G_CALLBACK(on_reports_clicked), dw);
    gtk_grid_attach(GTK_GRID(grid), dw->reports_button, 0, 1, 1, 1);
    
    gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, FALSE, 0);
    
    // Footer
    GtkWidget *footer_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(footer_label), 
                        "<span size='small' color='#888888'>Aakalan v1.0</span>");
    gtk_box_pack_end(GTK_BOX(main_box), footer_label, FALSE, FALSE, 10);
    
    gtk_container_add(GTK_CONTAINER(dw->window), main_box);
    
    g_signal_connect(dw->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Load classes and set initial button states
    refresh_class_dropdown(dw);
    update_button_states(dw);
    
    gtk_widget_show_all(dw->window);
}
