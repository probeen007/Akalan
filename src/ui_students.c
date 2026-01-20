#include "ui_students.h"
#include "database.h"
#include "validation.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *tree_view;
    GtkListStore *list_store;
    GtkWidget *name_entry;
    GtkWidget *email_entry;
    GtkWidget *roll_entry;
    GtkWidget *phone_entry;
    int selected_student_id;
} StudentsWindow;

enum {
    COL_ID = 0,
    COL_NAME,
    COL_EMAIL,
    COL_ROLL,
    COL_PHONE,
    NUM_COLS
};

// Enhanced CSS styling
static void apply_students_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        ".students-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 20px;"
        "  border-radius: 10px;"
        "  margin: 10px;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
        ".form-section {"
        "  background: #f8f9fa;"
        "  border-radius: 8px;"
        "  padding: 20px;"
        "  margin: 10px;"
        "  border: 1px solid #dee2e6;"
        "}"
        ".list-section {"
        "  background: white;"
        "  border-radius: 8px;"
        "  padding: 15px;"
        "  margin: 10px;"
        "  border: 1px solid #dee2e6;"
        "}"
        ".action-btn {"
        "  padding: 10px 20px;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  min-width: 100px;"
        "}"
        ".btn-add {"
        "  background: linear-gradient(135deg, #28a745 0%, #20c997 100%);"
        "  color: white;"
        "}"
        ".btn-update {"
        "  background: linear-gradient(135deg, #007bff 0%, #0056b3 100%);"
        "  color: white;"
        "}"
        ".btn-delete {"
        "  background: linear-gradient(135deg, #dc3545 0%, #c82333 100%);"
        "  color: white;"
        "}"
        ".btn-clear {"
        "  background: #6c757d;"
        "  color: white;"
        "}";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static void refresh_students_list(StudentsWindow *sw) {
    gtk_list_store_clear(sw->list_store);
    
    int count;
    Student **students = db_get_students_by_class(g_session.current_class_id, &count);
    
    if (students) {
        for (int i = 0; i < count; i++) {
            GtkTreeIter iter;
            gtk_list_store_append(sw->list_store, &iter);
            gtk_list_store_set(sw->list_store, &iter,
                             COL_ID, students[i]->id,
                             COL_NAME, students[i]->name,
                             COL_EMAIL, students[i]->email,
                             COL_ROLL, students[i]->roll_number,
                             COL_PHONE, students[i]->phone,
                             -1);
        }
        db_free_students(students, count);
    }
}

static void clear_form(StudentsWindow *sw) {
    gtk_entry_set_text(GTK_ENTRY(sw->name_entry), "");
    gtk_entry_set_text(GTK_ENTRY(sw->email_entry), "");
    gtk_entry_set_text(GTK_ENTRY(sw->roll_entry), "");
    gtk_entry_set_text(GTK_ENTRY(sw->phone_entry), "");
    sw->selected_student_id = 0;
}

static void on_add_student_clicked(GtkButton *button, gpointer user_data) {
    StudentsWindow *sw = (StudentsWindow *)user_data;
    
    const char *name = gtk_entry_get_text(GTK_ENTRY(sw->name_entry));
    const char *email = gtk_entry_get_text(GTK_ENTRY(sw->email_entry));
    const char *roll = gtk_entry_get_text(GTK_ENTRY(sw->roll_entry));
    const char *phone = gtk_entry_get_text(GTK_ENTRY(sw->phone_entry));
    
    // Validate
    if (!validate_required(name)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter student name.");
        return;
    }
    
    if (!validate_email(email)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter a valid email address.");
        return;
    }
    
    if (!validate_roll_number(roll)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter roll number.");
        return;
    }
    
    if (phone && strlen(phone) > 0 && !validate_phone(phone)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter a valid phone number.");
        return;
    }
    
    // Check for duplicates
    if (db_student_email_exists(email, 0)) {
        show_error_dialog(GTK_WINDOW(sw->window), "A student with this email already exists.");
        return;
    }
    
    if (db_student_roll_exists(roll, 0)) {
        show_error_dialog(GTK_WINDOW(sw->window), "A student with this roll number already exists.");
        return;
    }
    
    // Add to database
    if (db_create_student(name, email, roll, phone, g_session.current_class_id)) {
        show_info_dialog(GTK_WINDOW(sw->window), "Student added successfully!");
        clear_form(sw);
        refresh_students_list(sw);
    } else {
        show_error_dialog(GTK_WINDOW(sw->window), "Failed to add student. Please try again.");
    }
}

static void on_update_student_clicked(GtkButton *button, gpointer user_data) {
    StudentsWindow *sw = (StudentsWindow *)user_data;
    
    if (sw->selected_student_id == 0) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please select a student to update.");
        return;
    }
    
    const char *name = gtk_entry_get_text(GTK_ENTRY(sw->name_entry));
    const char *email = gtk_entry_get_text(GTK_ENTRY(sw->email_entry));
    const char *roll = gtk_entry_get_text(GTK_ENTRY(sw->roll_entry));
    const char *phone = gtk_entry_get_text(GTK_ENTRY(sw->phone_entry));
    
    // Validate
    if (!validate_required(name)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter student name.");
        return;
    }
    
    if (!validate_email(email)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter a valid email address.");
        return;
    }
    
    if (!validate_roll_number(roll)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter roll number.");
        return;
    }
    
    if (phone && strlen(phone) > 0 && !validate_phone(phone)) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please enter a valid phone number.");
        return;
    }
    
    // Check for duplicates (exclude current student)
    if (db_student_email_exists(email, sw->selected_student_id)) {
        show_error_dialog(GTK_WINDOW(sw->window), "A student with this email already exists.");
        return;
    }
    
    if (db_student_roll_exists(roll, sw->selected_student_id)) {
        show_error_dialog(GTK_WINDOW(sw->window), "A student with this roll number already exists.");
        return;
    }
    
    // Update in database
    if (db_update_student(sw->selected_student_id, name, email, roll, phone)) {
        show_info_dialog(GTK_WINDOW(sw->window), "Student updated successfully!");
        clear_form(sw);
        refresh_students_list(sw);
    } else {
        show_error_dialog(GTK_WINDOW(sw->window), "Failed to update student. Please try again.");
    }
}

static void on_delete_student_clicked(GtkButton *button, gpointer user_data) {
    StudentsWindow *sw = (StudentsWindow *)user_data;
    
    if (sw->selected_student_id == 0) {
        show_error_dialog(GTK_WINDOW(sw->window), "Please select a student to delete.");
        return;
    }
    
    if (!show_confirm_dialog(GTK_WINDOW(sw->window), 
                            "Are you sure you want to delete this student?\nThis will also delete all related attendance and assignment records.")) {
        return;
    }
    
    if (db_delete_student(sw->selected_student_id)) {
        show_info_dialog(GTK_WINDOW(sw->window), "Student deleted successfully!");
        clear_form(sw);
        refresh_students_list(sw);
    } else {
        show_error_dialog(GTK_WINDOW(sw->window), "Failed to delete student.");
    }
}

static void on_clear_form_clicked(GtkButton *button, gpointer user_data) {
    StudentsWindow *sw = (StudentsWindow *)user_data;
    clear_form(sw);
}

static void on_student_selected(GtkTreeView *tree_view, gpointer user_data) {
    StudentsWindow *sw = (StudentsWindow *)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        gchar *name, *email, *roll, *phone;
        
        gtk_tree_model_get(model, &iter,
                          COL_ID, &id,
                          COL_NAME, &name,
                          COL_EMAIL, &email,
                          COL_ROLL, &roll,
                          COL_PHONE, &phone,
                          -1);
        
        sw->selected_student_id = id;
        gtk_entry_set_text(GTK_ENTRY(sw->name_entry), name);
        gtk_entry_set_text(GTK_ENTRY(sw->email_entry), email);
        gtk_entry_set_text(GTK_ENTRY(sw->roll_entry), roll);
        gtk_entry_set_text(GTK_ENTRY(sw->phone_entry), phone ? phone : "");
        
        g_free(name);
        g_free(email);
        g_free(roll);
        g_free(phone);
    }
}

void show_students_window(GtkWindow *parent) {
    StudentsWindow *sw = g_malloc0(sizeof(StudentsWindow));
    sw->selected_student_id = 0;
    
    // Apply enhanced CSS
    apply_students_css();
    
    // Create window
    sw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(sw->window), "Student Management");
    gtk_window_set_default_size(GTK_WINDOW(sw->window), 1200, 700);
    gtk_window_set_position(GTK_WINDOW(sw->window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(sw->window), parent);
    gtk_container_set_border_width(GTK_CONTAINER(sw->window), 15);
    
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    // Header (no frame to avoid white border)
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 15);
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_ctx, "students-header");
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='x-large' weight='bold'>Student Management System</span>");
    gtk_box_pack_start(GTK_BOX(header_box), header, FALSE, FALSE, 0);
    
    GtkWidget *subtitle = gtk_label_new("Manage student records, enrollment, and information");
    gtk_box_pack_start(GTK_BOX(header_box), subtitle, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);
    
    // Paned for split view
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(paned), 650);
    
    // Left side - Student list
    GtkWidget *left_frame = gtk_frame_new(NULL);
    GtkStyleContext *left_ctx = gtk_widget_get_style_context(left_frame);
    gtk_style_context_add_class(left_ctx, "list-section");
    
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(left_box), 10);
    
    GtkWidget *list_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_label), "<b>Registered Students</b>");
    gtk_widget_set_halign(list_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left_box), list_label, FALSE, FALSE, 0);
    
    // Create tree view
    sw->list_store = gtk_list_store_new(NUM_COLS, G_TYPE_INT, G_TYPE_STRING, 
                                        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    sw->tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sw->list_store));
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(sw->tree_view), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
    
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // ID column (hidden)
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", COL_ID, NULL);
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(sw->tree_view), column);
    
    // Name column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COL_NAME, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(sw->tree_view), column);
    
    // Email column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Email", renderer, "text", COL_EMAIL, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(sw->tree_view), column);
    
    // Roll column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Roll No.", renderer, "text", COL_ROLL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(sw->tree_view), column);
    
    // Phone column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Phone", renderer, "text", COL_PHONE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(sw->tree_view), column);
    
    g_signal_connect(sw->tree_view, "cursor-changed", G_CALLBACK(on_student_selected), sw);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), sw->tree_view);
    gtk_box_pack_start(GTK_BOX(left_box), scrolled, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(left_frame), left_box);
    gtk_paned_pack1(GTK_PANED(paned), left_frame, TRUE, TRUE);
    
    // Right side - Form
    GtkWidget *right_frame = gtk_frame_new(NULL);
    GtkStyleContext *right_ctx = gtk_widget_get_style_context(right_frame);
    gtk_style_context_add_class(right_ctx, "form-section");
    
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(right_box), 15);
    gtk_widget_set_size_request(right_box, 380, -1);
    
    GtkWidget *form_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(form_label), "<b>Student Information</b>");
    gtk_widget_set_halign(form_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), form_label, FALSE, FALSE, 0);
    
    // Form fields
    GtkWidget *name_label = gtk_label_new("Full Name:");
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), name_label, FALSE, FALSE, 0);
    
    sw->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(sw->name_entry), "Enter student name");
    gtk_box_pack_start(GTK_BOX(right_box), sw->name_entry, FALSE, FALSE, 0);
    
    GtkWidget *email_label = gtk_label_new("Email:");
    gtk_widget_set_halign(email_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), email_label, FALSE, FALSE, 0);
    
    sw->email_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(sw->email_entry), "student@example.com");
    gtk_box_pack_start(GTK_BOX(right_box), sw->email_entry, FALSE, FALSE, 0);
    
    GtkWidget *roll_label = gtk_label_new("Roll Number:");
    gtk_widget_set_halign(roll_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), roll_label, FALSE, FALSE, 0);
    
    sw->roll_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(sw->roll_entry), "Enter roll number");
    gtk_box_pack_start(GTK_BOX(right_box), sw->roll_entry, FALSE, FALSE, 0);
    
    GtkWidget *phone_label = gtk_label_new("Phone (Optional):");
    gtk_widget_set_halign(phone_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), phone_label, FALSE, FALSE, 0);
    
    sw->phone_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(sw->phone_entry), "Enter phone number");
    gtk_box_pack_start(GTK_BOX(right_box), sw->phone_entry, FALSE, FALSE, 0);
    
    // Buttons with styling
    GtkWidget *button_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(button_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(button_grid), 8);
    gtk_widget_set_margin_top(button_grid, 15);
    
    GtkWidget *add_button = gtk_button_new_with_label("Add Student");
    GtkStyleContext *add_ctx = gtk_widget_get_style_context(add_button);
    gtk_style_context_add_class(add_ctx, "action-btn");
    gtk_style_context_add_class(add_ctx, "btn-add");
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_student_clicked), sw);
    gtk_grid_attach(GTK_GRID(button_grid), add_button, 0, 0, 1, 1);
    
    GtkWidget *update_button = gtk_button_new_with_label("Update");
    GtkStyleContext *upd_ctx = gtk_widget_get_style_context(update_button);
    gtk_style_context_add_class(upd_ctx, "action-btn");
    gtk_style_context_add_class(upd_ctx, "btn-update");
    g_signal_connect(update_button, "clicked", G_CALLBACK(on_update_student_clicked), sw);
    gtk_grid_attach(GTK_GRID(button_grid), update_button, 1, 0, 1, 1);
    
    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    GtkStyleContext *del_ctx = gtk_widget_get_style_context(delete_button);
    gtk_style_context_add_class(del_ctx, "action-btn");
    gtk_style_context_add_class(del_ctx, "btn-delete");
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_student_clicked), sw);
    gtk_grid_attach(GTK_GRID(button_grid), delete_button, 0, 1, 1, 1);
    
    GtkWidget *clear_button = gtk_button_new_with_label("Clear Form");
    GtkStyleContext *clr_ctx = gtk_widget_get_style_context(clear_button);
    gtk_style_context_add_class(clr_ctx, "action-btn");
    gtk_style_context_add_class(clr_ctx, "btn-clear");
    g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_form_clicked), sw);
    gtk_grid_attach(GTK_GRID(button_grid), clear_button, 1, 1, 1, 1);
    
    gtk_box_pack_start(GTK_BOX(right_box), button_grid, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(right_frame), right_box);
    gtk_paned_pack2(GTK_PANED(paned), right_frame, FALSE, TRUE);
    
    gtk_box_pack_start(GTK_BOX(main_box), paned, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(sw->window), main_box);
    
    // Load initial data
    refresh_students_list(sw);
    
    gtk_widget_show_all(sw->window);
}
