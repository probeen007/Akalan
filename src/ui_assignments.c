#include "ui_assignments.h"
#include "database.h"
#include "validation.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    
    // Assignment List Tab
    GtkWidget *tree_view;
    GtkListStore *list_store;
    
    // Create Assignment Tab
    GtkWidget *title_entry;
    GtkWidget *subject_entry;
    GtkWidget *description_text;
    GtkWidget *due_date_entry;
    GtkWidget *due_time_entry;
    
    // Track Submissions Tab
    GtkWidget *assignment_combo;
    GtkWidget *tracking_header;
    GtkWidget *submissions_box;
    GtkWidget *save_all_btn;
    int selected_assignment_id;
    int has_unsaved_changes;
} AssignmentsWindow;

enum {
    COL_ASSIGN_ID = 0,
    COL_ASSIGN_TITLE,
    COL_ASSIGN_SUBJECT,
    COL_ASSIGN_DUE_DATE,
    NUM_ASSIGN_COLS
};

static void refresh_assignments_list(AssignmentsWindow *aw) {
    gtk_list_store_clear(aw->list_store);
    
    int count;
    Assignment **assignments = db_get_assignments_by_class(g_session.current_class_id, &count);
    
    if (assignments) {
        for (int i = 0; i < count; i++) {
            char due_date_str[50];
            format_datetime(assignments[i]->due_date, due_date_str, sizeof(due_date_str));
            
            GtkTreeIter iter;
            gtk_list_store_append(aw->list_store, &iter);
            gtk_list_store_set(aw->list_store, &iter,
                             COL_ASSIGN_ID, assignments[i]->id,
                             COL_ASSIGN_TITLE, assignments[i]->title,
                             COL_ASSIGN_SUBJECT, assignments[i]->subject,
                             COL_ASSIGN_DUE_DATE, due_date_str,
                             -1);
        }
        db_free_assignments(assignments, count);
    }
}

static void on_create_assignment_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AssignmentsWindow *aw = (AssignmentsWindow *)user_data;
    
    const char *title = gtk_entry_get_text(GTK_ENTRY(aw->title_entry));
    const char *subject = gtk_entry_get_text(GTK_ENTRY(aw->subject_entry));
    const char *date_str = gtk_entry_get_text(GTK_ENTRY(aw->due_date_entry));
    const char *time_str = gtk_entry_get_text(GTK_ENTRY(aw->due_time_entry));
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(aw->description_text));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    // Validate
    if (!validate_required(title)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please enter assignment title.");
        g_free(description);
        return;
    }
    
    if (!validate_required(subject)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please enter subject.");
        g_free(description);
        return;
    }
    
    if (!validate_date(date_str)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please enter valid date (YYYY-MM-DD).");
        g_free(description);
        return;
    }
    
    if (!validate_time(time_str)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please enter valid time (HH:MM).");
        g_free(description);
        return;
    }
    
    // Parse datetime
    char datetime_str[100];
    snprintf(datetime_str, sizeof(datetime_str), "%s %s:00", date_str, time_str);
    
    time_t due_date;
    if (!parse_datetime(datetime_str, &due_date)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Failed to parse date/time.");
        g_free(description);
        return;
    }
    
    // Create assignment
    if (db_create_assignment(title, subject, description, due_date, g_session.user_id, g_session.current_class_id)) {
        show_info_dialog(GTK_WINDOW(aw->window), "Assignment created successfully!");
        
        // Clear form
        gtk_entry_set_text(GTK_ENTRY(aw->title_entry), "");
        gtk_entry_set_text(GTK_ENTRY(aw->subject_entry), "");
        gtk_entry_set_text(GTK_ENTRY(aw->due_date_entry), "");
        gtk_entry_set_text(GTK_ENTRY(aw->due_time_entry), "");
        gtk_text_buffer_set_text(buffer, "", -1);
        
        // Refresh list
        refresh_assignments_list(aw);
        
        // Switch to list tab
        gtk_notebook_set_current_page(GTK_NOTEBOOK(aw->notebook), 0);
    } else {
        show_error_dialog(GTK_WINDOW(aw->window), "Failed to create assignment.");
    }
    
    g_free(description);
}

static void on_delete_assignment_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AssignmentsWindow *aw = (AssignmentsWindow *)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(aw->tree_view));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_error_dialog(GTK_WINDOW(aw->window), "Please select an assignment to delete.");
        return;
    }
    
    gint id;
    gchar *title;
    gtk_tree_model_get(model, &iter, COL_ASSIGN_ID, &id, COL_ASSIGN_TITLE, &title, -1);
    
    char message[256];
    snprintf(message, sizeof(message), "Are you sure you want to delete '%s'?\nThis will also delete all submission records.", title);
    
    if (show_confirm_dialog(GTK_WINDOW(aw->window), message)) {
        if (db_delete_assignment(id)) {
            show_info_dialog(GTK_WINDOW(aw->window), "Assignment deleted successfully!");
            refresh_assignments_list(aw);
        } else {
            show_error_dialog(GTK_WINDOW(aw->window), "Failed to delete assignment.");
        }
    }
    
    g_free(title);
}

static void on_submission_status_changed(GtkComboBox *combo, gpointer user_data) {
    (void)user_data;
    int student_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(combo), "student_id"));
    int assignment_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(combo), "assignment_id"));
    GtkWidget *quality_combo = GTK_WIDGET(g_object_get_data(G_OBJECT(combo), "quality_combo"));
    GtkWidget *notes_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(combo), "notes_entry"));
    
    int status = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    int quality = gtk_combo_box_get_active(GTK_COMBO_BOX(quality_combo));
    const char *notes = gtk_entry_get_text(GTK_ENTRY(notes_entry));
    
    db_create_or_update_submission(assignment_id, student_id, 
                                   (SubmissionStatus)status, 
                                   (QualityAssessment)quality, 
                                   notes);
}

static void on_submission_quality_changed(GtkComboBox *combo, gpointer user_data) {
    (void)combo;
    GtkWidget *status_combo = GTK_WIDGET(user_data);
    on_submission_status_changed(GTK_COMBO_BOX(status_combo), NULL);
}

static void on_submission_notes_changed(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    GtkWidget *status_combo = GTK_WIDGET(user_data);
    on_submission_status_changed(GTK_COMBO_BOX(status_combo), NULL);
}

static void on_save_all_submissions_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AssignmentsWindow *aw = (AssignmentsWindow *)user_data;
    
    // Trigger save for all entries
    GList *children = gtk_container_get_children(GTK_CONTAINER(aw->submissions_box));
    int saved_count = 0;
    
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *frame = GTK_WIDGET(iter->data);
        GList *frame_children = gtk_container_get_children(GTK_CONTAINER(frame));
        if (frame_children) {
            GtkWidget *hbox = GTK_WIDGET(frame_children->data);
            GList *hbox_children = gtk_container_get_children(GTK_CONTAINER(hbox));
            
            // Find the status combo (second widget after label)
            if (hbox_children && hbox_children->next) {
                GtkWidget *status_combo = GTK_WIDGET(hbox_children->next->data);
                on_submission_status_changed(GTK_COMBO_BOX(status_combo), NULL);
                saved_count++;
            }
            
            g_list_free(hbox_children);
            g_list_free(frame_children);
        }
    }
    g_list_free(children);
    
    char message[100];
    snprintf(message, sizeof(message), "Saved %d student submission records!", saved_count);
    show_info_dialog(GTK_WINDOW(aw->window), message);
    
    aw->has_unsaved_changes = 0;
    gtk_widget_set_sensitive(aw->save_all_btn, FALSE);
}

static void mark_unsaved_changes(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    AssignmentsWindow *aw = (AssignmentsWindow *)user_data;
    aw->has_unsaved_changes = 1;
    if (aw->save_all_btn) {
        gtk_widget_set_sensitive(aw->save_all_btn, TRUE);
    }
}

static void load_submissions_for_assignment(AssignmentsWindow *aw, int assignment_id) {
    // Clear existing widgets
    GList *children = gtk_container_get_children(GTK_CONTAINER(aw->submissions_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Update header with assignment details
    Assignment *assignment = db_get_assignment_by_id(assignment_id);
    if (assignment) {
        char due_date_str[50];
        format_datetime(assignment->due_date, due_date_str, sizeof(due_date_str));
        
        char header_text[500];
        snprintf(header_text, sizeof(header_text), 
                "<span font='14' weight='bold'>Tracking: %s</span>\n"
                "<span font='11'>Subject: %s | Due: %s</span>",
                assignment->title, assignment->subject, due_date_str);
        
        gtk_label_set_markup(GTK_LABEL(aw->tracking_header), header_text);
        free(assignment);
    }
    
    // Get students from current class
    int student_count;
    Student **students = db_get_students_by_class(g_session.current_class_id, &student_count);
    
    if (!students || student_count == 0) {
        GtkWidget *label = gtk_label_new("No students found. Please add students first.");
        gtk_box_pack_start(GTK_BOX(aw->submissions_box), label, FALSE, FALSE, 10);
        gtk_widget_show_all(aw->submissions_box);
        gtk_widget_set_sensitive(aw->save_all_btn, FALSE);
        return;
    }
    
    // Create submission widgets for each student
    for (int i = 0; i < student_count; i++) {
        GtkWidget *frame = gtk_frame_new(NULL);
        GtkStyleContext *student_frame_ctx = gtk_widget_get_style_context(frame);
        gtk_style_context_add_class(student_frame_ctx, "student-frame");
        
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
        
        // Student name header
        GtkWidget *name_label = gtk_label_new(NULL);
        char name_markup[256];
        snprintf(name_markup, sizeof(name_markup), "<b>%s</b> <small>(%s)</small>", 
                students[i]->name, students[i]->roll_number);
        gtk_label_set_markup(GTK_LABEL(name_label), name_markup);
        gtk_widget_set_halign(name_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), name_label, FALSE, FALSE, 0);
        
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
        
        // Status combo
        GtkWidget *status_label = gtk_label_new("Status:");
        GtkWidget *status_combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "No Submission");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Timely Submission");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Late Submission");
        
        // Quality combo
        GtkWidget *quality_label = gtk_label_new("Quality:");
        GtkWidget *quality_combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(quality_combo), "Poor");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(quality_combo), "Below Average");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(quality_combo), "Above Average");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(quality_combo), "High");
        
        // Notes entry
        GtkWidget *notes_label = gtk_label_new("Notes:");
        GtkWidget *notes_entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(notes_entry), "Optional notes");
        gtk_widget_set_size_request(notes_entry, 200, -1);
        
        // Load existing submission if any
        AssignmentSubmission *sub = db_get_submission(assignment_id, students[i]->id);
        if (sub) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), sub->status);
            gtk_combo_box_set_active(GTK_COMBO_BOX(quality_combo), sub->quality);
            gtk_entry_set_text(GTK_ENTRY(notes_entry), sub->notes);
            free(sub);
        } else {
            gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
            gtk_combo_box_set_active(GTK_COMBO_BOX(quality_combo), 0);
        }
        
        // Store student ID in widgets
        g_object_set_data(G_OBJECT(status_combo), "student_id", GINT_TO_POINTER(students[i]->id));
        g_object_set_data(G_OBJECT(status_combo), "quality_combo", quality_combo);
        g_object_set_data(G_OBJECT(status_combo), "notes_entry", notes_entry);
        g_object_set_data(G_OBJECT(status_combo), "assignment_id", GINT_TO_POINTER(assignment_id));
        
        // Connect signals for tracking changes
        g_signal_connect(status_combo, "changed", G_CALLBACK(on_submission_status_changed), NULL);
        g_signal_connect(status_combo, "changed", G_CALLBACK(mark_unsaved_changes), aw);
        
        g_signal_connect(quality_combo, "changed", G_CALLBACK(on_submission_quality_changed), status_combo);
        g_signal_connect(quality_combo, "changed", G_CALLBACK(mark_unsaved_changes), aw);
        
        // Use "changed" signal for notes to auto-save as user types
        g_signal_connect(notes_entry, "changed", G_CALLBACK(on_submission_notes_changed), status_combo);
        g_signal_connect(notes_entry, "changed", G_CALLBACK(mark_unsaved_changes), aw);
        
        gtk_box_pack_start(GTK_BOX(hbox), status_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), status_combo, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), quality_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), quality_combo, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), notes_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), notes_entry, TRUE, TRUE, 0);
        
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_box_pack_start(GTK_BOX(aw->submissions_box), frame, FALSE, FALSE, 6);
    }
    
    db_free_students(students, student_count);
    gtk_widget_show_all(aw->submissions_box);
    
    // Reset unsaved changes flag
    aw->has_unsaved_changes = 0;
    gtk_widget_set_sensitive(aw->save_all_btn, FALSE);
}

static void on_assignment_selected_for_tracking(GtkComboBox *combo, gpointer user_data) {
    AssignmentsWindow *aw = (AssignmentsWindow *)user_data;
    
    int active = gtk_combo_box_get_active(combo);
    if (active < 0) {
        gtk_label_set_text(GTK_LABEL(aw->tracking_header), "Select an assignment to track submissions");
        return;
    }
    
    // Get assignment ID from combo box
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_combo_box_get_model(combo);
    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        gint id;
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        aw->selected_assignment_id = id;
        load_submissions_for_assignment(aw, id);
    }
}

static void populate_assignment_combo(AssignmentsWindow *aw) {
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
    
    int count;
    Assignment **assignments = db_get_assignments_by_class(g_session.current_class_id, &count);
    
    if (assignments) {
        for (int i = 0; i < count; i++) {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 
                             0, assignments[i]->id,
                             1, assignments[i]->title,
                             -1);
        }
        db_free_assignments(assignments, count);
    }
    
    gtk_combo_box_set_model(GTK_COMBO_BOX(aw->assignment_combo), GTK_TREE_MODEL(store));
    
    // Set up cell renderer to display the title (column 1)
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_clear(GTK_CELL_LAYOUT(aw->assignment_combo));
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(aw->assignment_combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(aw->assignment_combo), renderer, "text", 1, NULL);
    
    g_object_unref(store);
}

void show_assignments_window(GtkWindow *parent) {
    AssignmentsWindow *aw = g_malloc0(sizeof(AssignmentsWindow));
    aw->selected_assignment_id = 0;
    aw->has_unsaved_changes = 0;
    
    // Create window
    aw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(aw->window), "Assignment Tracking");
    gtk_window_set_default_size(GTK_WINDOW(aw->window), 1100, 680);
    gtk_window_set_position(GTK_WINDOW(aw->window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(aw->window), parent);
    gtk_container_set_border_width(GTK_CONTAINER(aw->window), 15);
    
    // Apply modern CSS styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        ".assignment-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 20px;"
        "  border-radius: 10px;"
        "  margin: 5px;"
        "}"
        ".tab-section {"
        "  background: #f8f9fa;"
        "  border-radius: 10px;"
        "  padding: 20px;"
        "  margin: 10px;"
        "}"
        ".form-card {"
        "  background: white;"
        "  border: 1px solid #e0e0e0;"
        "  border-radius: 8px;"
        "  padding: 20px;"
        "  margin: 10px 0;"
        "  box-shadow: 0 2px 4px rgba(0,0,0,0.1);"
        "}"
        ".section-title {"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  color: #2c3e50;"
        "  padding: 10px 0;"
        "}"
        ".form-label {"
        "  font-weight: 600;"
        "  color: #495057;"
        "  padding: 5px 0;"
        "}"
        ".action-button {"
        "  padding: 10px 20px;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  min-width: 120px;"
        "}"
        ".primary-button {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "}"
        ".success-button {"
        "  background: linear-gradient(135deg, #28a745 0%, #20c997 100%);"
        "  color: white;"
        "}"
        ".danger-button {"
        "  background: linear-gradient(135deg, #dc3545 0%, #c82333 100%);"
        "  color: white;"
        "}"
        ".student-frame {"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 6px;"
        "  margin: 5px;"
        "  padding: 12px;"
        "  background: #ffffff;"
        "}"
        ".tracking-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 15px;"
        "  border-radius: 8px;"
        "  font-weight: bold;"
        "}",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css_provider);
    
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    // Header (no frame to avoid white border)
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 15);
    GtkStyleContext *header_context = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_context, "assignment-header");
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='x-large' weight='bold'>Assignment Tracking System</span>");
    gtk_widget_set_halign(header, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(header_box), header, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);
    
    // Notebook with tabs
    aw->notebook = gtk_notebook_new();
    
    // === TAB 1: Assignment List ===
    GtkWidget *list_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(list_tab), 15);
    
    GtkWidget *list_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    GtkStyleContext *list_card_ctx = gtk_widget_get_style_context(list_card);
    gtk_style_context_add_class(list_card_ctx, "form-card");
    
    GtkWidget *list_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_header), "<span size='large' weight='bold'>All Assignments</span>");
    gtk_widget_set_halign(list_header, GTK_ALIGN_START);
    GtkStyleContext *lh_ctx = gtk_widget_get_style_context(list_header);
    gtk_style_context_add_class(lh_ctx, "section-title");
    gtk_box_pack_start(GTK_BOX(list_card), list_header, FALSE, FALSE, 0);
    
    // Toolbar with styled buttons
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete Selected");
    gtk_widget_set_size_request(delete_btn, 160, 40);
    GtkStyleContext *del_ctx = gtk_widget_get_style_context(delete_btn);
    gtk_style_context_add_class(del_ctx, "action-button");
    gtk_style_context_add_class(del_ctx, "danger-button");
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_assignment_clicked), aw);
    gtk_box_pack_start(GTK_BOX(toolbar), delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(list_card), toolbar, FALSE, FALSE, 0);
    
    // Tree view
    aw->list_store = gtk_list_store_new(NUM_ASSIGN_COLS, G_TYPE_INT, G_TYPE_STRING, 
                                        G_TYPE_STRING, G_TYPE_STRING);
    aw->tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(aw->list_store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(aw->tree_view),
        gtk_tree_view_column_new_with_attributes("Title", renderer, "text", COL_ASSIGN_TITLE, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(aw->tree_view),
        gtk_tree_view_column_new_with_attributes("Subject", renderer, "text", COL_ASSIGN_SUBJECT, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(aw->tree_view),
        gtk_tree_view_column_new_with_attributes("Due Date", renderer, "text", COL_ASSIGN_DUE_DATE, NULL));
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled, -1, 400);
    gtk_container_add(GTK_CONTAINER(scrolled), aw->tree_view);
    gtk_box_pack_start(GTK_BOX(list_card), scrolled, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(list_tab), list_card, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(aw->notebook), list_tab, gtk_label_new("Assignments"));
    
    // === TAB 2: Create Assignment ===
    GtkWidget *create_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(create_tab), 15);
    
    GtkWidget *create_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    GtkStyleContext *create_card_ctx = gtk_widget_get_style_context(create_card);
    gtk_style_context_add_class(create_card_ctx, "form-card");
    
    GtkWidget *create_header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(create_header), "<span size='large' weight='bold'>Create New Assignment</span>");
    gtk_widget_set_halign(create_header, GTK_ALIGN_START);
    GtkStyleContext *ch_ctx = gtk_widget_get_style_context(create_header);
    gtk_style_context_add_class(ch_ctx, "section-title");
    gtk_box_pack_start(GTK_BOX(create_card), create_header, FALSE, FALSE, 0);
    
    // Title field
    GtkWidget *title_label = gtk_label_new("Title:");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    GtkStyleContext *tl_ctx = gtk_widget_get_style_context(title_label);
    gtk_style_context_add_class(tl_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(create_card), title_label, FALSE, FALSE, 0);
    aw->title_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(aw->title_entry), "Enter assignment title");
    gtk_widget_set_size_request(aw->title_entry, -1, 35);
    gtk_box_pack_start(GTK_BOX(create_card), aw->title_entry, FALSE, FALSE, 0);
    
    // Subject field
    GtkWidget *subject_label = gtk_label_new("Subject:");
    gtk_widget_set_halign(subject_label, GTK_ALIGN_START);
    GtkStyleContext *sl_ctx = gtk_widget_get_style_context(subject_label);
    gtk_style_context_add_class(sl_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(create_card), subject_label, FALSE, FALSE, 0);
    aw->subject_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(aw->subject_entry), "Enter subject name");
    gtk_widget_set_size_request(aw->subject_entry, -1, 35);
    gtk_box_pack_start(GTK_BOX(create_card), aw->subject_entry, FALSE, FALSE, 0);
    
    // Description field
    GtkWidget *desc_label = gtk_label_new("Description:");
    gtk_widget_set_halign(desc_label, GTK_ALIGN_START);
    GtkStyleContext *dl_ctx = gtk_widget_get_style_context(desc_label);
    gtk_style_context_add_class(dl_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(create_card), desc_label, FALSE, FALSE, 0);
    aw->description_text = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(aw->description_text), GTK_WRAP_WORD);
    GtkWidget *desc_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(desc_scroll, -1, 120);
    gtk_container_add(GTK_CONTAINER(desc_scroll), aw->description_text);
    gtk_box_pack_start(GTK_BOX(create_card), desc_scroll, FALSE, FALSE, 0);
    
    // Due date and time
    GtkWidget *due_label = gtk_label_new("Due Date & Time:");
    gtk_widget_set_halign(due_label, GTK_ALIGN_START);
    GtkStyleContext *dul_ctx = gtk_widget_get_style_context(due_label);
    gtk_style_context_add_class(dul_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(create_card), due_label, FALSE, FALSE, 0);
    
    GtkWidget *date_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    GtkWidget *date_label = gtk_label_new("Date (YYYY-MM-DD):");
    aw->due_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(aw->due_date_entry), "2026-01-31");
    gtk_widget_set_size_request(aw->due_date_entry, 150, 35);
    GtkWidget *time_label = gtk_label_new("Time (HH:MM):");
    aw->due_time_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(aw->due_time_entry), "23:59");
    gtk_widget_set_size_request(aw->due_time_entry, 100, 35);
    gtk_box_pack_start(GTK_BOX(date_box), date_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(date_box), aw->due_date_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(date_box), time_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(date_box), aw->due_time_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(create_card), date_box, FALSE, FALSE, 0);
    
    // Create button
    GtkWidget *create_btn = gtk_button_new_with_label("Create Assignment");
    gtk_widget_set_size_request(create_btn, 200, 45);
    GtkStyleContext *cb_ctx = gtk_widget_get_style_context(create_btn);
    gtk_style_context_add_class(cb_ctx, "action-button");
    gtk_style_context_add_class(cb_ctx, "success-button");
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_assignment_clicked), aw);
    gtk_widget_set_halign(create_btn, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(create_card), create_btn, FALSE, FALSE, 10);
    
    gtk_box_pack_start(GTK_BOX(create_tab), create_card, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(aw->notebook), create_tab, gtk_label_new("Create New"));
    
    // === TAB 3: Track Submissions ===
    GtkWidget *track_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(track_tab), 15);
    
    // Selection section
    GtkWidget *select_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkStyleContext *sel_card_ctx = gtk_widget_get_style_context(select_card);
    gtk_style_context_add_class(sel_card_ctx, "form-card");
    
    GtkWidget *select_label = gtk_label_new("Select Assignment:");
    gtk_widget_set_halign(select_label, GTK_ALIGN_START);
    GtkStyleContext *sell_ctx = gtk_widget_get_style_context(select_label);
    gtk_style_context_add_class(sell_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(select_card), select_label, FALSE, FALSE, 0);
    
    aw->assignment_combo = gtk_combo_box_new();
    gtk_widget_set_size_request(aw->assignment_combo, -1, 35);
    g_signal_connect(aw->assignment_combo, "changed", 
                    G_CALLBACK(on_assignment_selected_for_tracking), aw);
    gtk_box_pack_start(GTK_BOX(select_card), aw->assignment_combo, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(track_tab), select_card, FALSE, FALSE, 0);
    
    // Tracking header - shows which assignment is being tracked
    aw->tracking_header = gtk_label_new("Select an assignment to track submissions");
    gtk_label_set_line_wrap(GTK_LABEL(aw->tracking_header), TRUE);
    gtk_widget_set_halign(aw->tracking_header, GTK_ALIGN_START);
    GtkStyleContext *th_ctx = gtk_widget_get_style_context(aw->tracking_header);
    gtk_style_context_add_class(th_ctx, "tracking-header");
    gtk_box_pack_start(GTK_BOX(track_tab), aw->tracking_header, FALSE, FALSE, 0);
    
    // Save button
    GtkWidget *save_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    aw->save_all_btn = gtk_button_new_with_label("Save All Submissions");
    gtk_widget_set_size_request(aw->save_all_btn, 220, 40);
    gtk_widget_set_sensitive(aw->save_all_btn, FALSE);
    GtkStyleContext *sab_ctx = gtk_widget_get_style_context(aw->save_all_btn);
    gtk_style_context_add_class(sab_ctx, "action-button");
    gtk_style_context_add_class(sab_ctx, "primary-button");
    g_signal_connect(aw->save_all_btn, "clicked", G_CALLBACK(on_save_all_submissions_clicked), aw);
    gtk_box_pack_start(GTK_BOX(save_box), aw->save_all_btn, FALSE, FALSE, 0);
    
    GtkWidget *info_label = gtk_label_new("ℹ️ Changes are auto-saved");
    gtk_widget_set_halign(info_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(save_box), info_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(track_tab), save_box, FALSE, FALSE, 0);
    
    GtkWidget *track_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(track_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(track_scroll, -1, 300);
    
    aw->submissions_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(track_scroll), aw->submissions_box);
    gtk_box_pack_start(GTK_BOX(track_tab), track_scroll, TRUE, TRUE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(aw->notebook), track_tab, gtk_label_new("Track Submissions"));
    
    gtk_box_pack_start(GTK_BOX(main_box), aw->notebook, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(aw->window), main_box);
    
    // Load initial data
    refresh_assignments_list(aw);
    populate_assignment_combo(aw);
    
    gtk_widget_show_all(aw->window);
}
