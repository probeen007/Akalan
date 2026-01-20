#include "ui_classes.h"
#include "database.h"
#include "validation.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *name_entry;
    GtkWidget *description_text;
    void (*on_class_created_callback)(void);
} CreateClassWindow;

static void on_create_class_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    CreateClassWindow *cw = (CreateClassWindow *)user_data;
    
    const char *name = gtk_entry_get_text(GTK_ENTRY(cw->name_entry));
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->description_text));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    // Validate
    if (!validate_required(name)) {
        show_error_dialog(GTK_WINDOW(cw->window), "Please enter class name.");
        g_free(description);
        return;
    }
    
    if (strlen(name) < 3) {
        show_error_dialog(GTK_WINDOW(cw->window), "Class name must be at least 3 characters long.");
        g_free(description);
        return;
    }
    
    // Create class
    int class_id = 0;
    if (db_create_class(name, description, g_session.user_id, &class_id)) {
        show_info_dialog(GTK_WINDOW(cw->window), "Class created successfully!");
        
        // Call callback to refresh dropdown
        if (cw->on_class_created_callback) {
            cw->on_class_created_callback();
        }
        
        gtk_widget_destroy(cw->window);
    } else {
        show_error_dialog(GTK_WINDOW(cw->window), "Failed to create class. Please try again.");
    }
    
    g_free(description);
}

void show_create_class_window(GtkWindow *parent, void (*on_class_created_callback)(void)) {
    CreateClassWindow *cw = g_malloc(sizeof(CreateClassWindow));
    cw->on_class_created_callback = on_class_created_callback;
    
    // Create window
    cw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(cw->window), "Create New Class");
    gtk_window_set_default_size(GTK_WINDOW(cw->window), 500, 400);
    gtk_window_set_position(GTK_WINDOW(cw->window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(cw->window), parent);
    gtk_window_set_modal(GTK_WINDOW(cw->window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(cw->window), 20);
    
    // Apply CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        ".class-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 20px;"
        "  border-radius: 10px;"
        "  margin-bottom: 20px;"
        "}"
        ".form-label {"
        "  font-weight: 600;"
        "  color: #495057;"
        "  padding: 8px 0;"
        "}"
        ".create-btn {"
        "  background: linear-gradient(135deg, #28a745 0%, #20c997 100%);"
        "  color: white;"
        "  padding: 12px 30px;"
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
    
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    
    // Header
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_ctx, "class-header");
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), 
        "<span size='x-large' weight='bold'>Create New Class</span>");
    gtk_box_pack_start(GTK_BOX(header_box), header, FALSE, FALSE, 0);
    
    GtkWidget *subtitle = gtk_label_new("Set up a new class to organize students and assignments");
    gtk_box_pack_start(GTK_BOX(header_box), subtitle, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);
    
    // Form section
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    // Class name
    GtkWidget *name_label = gtk_label_new("Class Name:");
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    GtkStyleContext *nl_ctx = gtk_widget_get_style_context(name_label);
    gtk_style_context_add_class(nl_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(form_box), name_label, FALSE, FALSE, 0);
    
    cw->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(cw->name_entry), "e.g., Computer Science 101, Mathematics Grade 10");
    gtk_widget_set_size_request(cw->name_entry, -1, 40);
    gtk_box_pack_start(GTK_BOX(form_box), cw->name_entry, FALSE, FALSE, 0);
    
    // Description
    GtkWidget *desc_label = gtk_label_new("Description (Optional):");
    gtk_widget_set_halign(desc_label, GTK_ALIGN_START);
    GtkStyleContext *dl_ctx = gtk_widget_get_style_context(desc_label);
    gtk_style_context_add_class(dl_ctx, "form-label");
    gtk_box_pack_start(GTK_BOX(form_box), desc_label, FALSE, FALSE, 0);
    
    cw->description_text = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cw->description_text), GTK_WRAP_WORD);
    GtkWidget *desc_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(desc_scroll, -1, 100);
    gtk_container_add(GTK_CONTAINER(desc_scroll), cw->description_text);
    gtk_box_pack_start(GTK_BOX(form_box), desc_scroll, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), form_box, TRUE, TRUE, 0);
    
    // Button section
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    
    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    gtk_widget_set_size_request(cancel_btn, 100, 40);
    g_signal_connect_swapped(cancel_btn, "clicked", 
                             G_CALLBACK(gtk_widget_destroy), cw->window);
    gtk_box_pack_start(GTK_BOX(button_box), cancel_btn, FALSE, FALSE, 0);
    
    GtkWidget *create_btn = gtk_button_new_with_label("Create Class");
    gtk_widget_set_size_request(create_btn, 150, 40);
    GtkStyleContext *cb_ctx = gtk_widget_get_style_context(create_btn);
    gtk_style_context_add_class(cb_ctx, "create-btn");
    g_signal_connect(create_btn, "clicked", 
                    G_CALLBACK(on_create_class_clicked), cw);
    gtk_box_pack_start(GTK_BOX(button_box), create_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(cw->window), main_box);
    
    // Focus on name entry
    gtk_widget_grab_focus(cw->name_entry);
    
    gtk_widget_show_all(cw->window);
}
