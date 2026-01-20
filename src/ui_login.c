#include "ui_login.h"
#include "ui_dashboard.h"
#include "database.h"
#include "security.h"
#include "validation.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *login_email_entry;
    GtkWidget *login_password_entry;
    GtkWidget *reg_email_entry;
    GtkWidget *reg_password_entry;
    GtkWidget *name_entry;
    GtkWidget *login_box;
    GtkWidget *register_box;
    GtkWidget *main_stack;
} LoginWindow;

// Apply modern CSS styling
static void apply_login_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        ".login-header {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  padding: 30px;"
        "  border-radius: 10px 10px 0 0;"
        "}"
        ".login-container {"
        "  background: white;"
        "  border-radius: 10px;"
        "  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.15);"
        "}"
        ".login-input {"
        "  border: 2px solid #e0e0e0;"
        "  border-radius: 6px;"
        "  padding: 10px;"
        "  min-height: 40px;"
        "}"
        ".login-input:focus {"
        "  border-color: #667eea;"
        "  box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);"
        "}"
        ".login-btn {"
        "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 12px;"
        "  font-weight: bold;"
        "  min-height: 45px;"
        "}"
        ".login-btn:hover {"
        "  background: linear-gradient(135deg, #5568d3 0%, #653a8b 100%);"
        "}"
        ".link-btn {"
        "  color: #667eea;"
        "  border: none;"
        "  background: none;"
        "}"
        ".link-btn:hover {"
        "  color: #5568d3;"
        "}";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static void on_login_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    LoginWindow *lw = (LoginWindow *)user_data;
    
    const char *email = gtk_entry_get_text(GTK_ENTRY(lw->login_email_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(lw->login_password_entry));
    
    // Validate inputs
    if (!validate_email(email)) {
        show_error_dialog(GTK_WINDOW(lw->window), "Please enter a valid email address.");
        return;
    }
    
    if (!validate_password(password, 8)) {
        show_error_dialog(GTK_WINDOW(lw->window), "Password must be at least 8 characters and contain at least one digit or special character.");
        return;
    }
    
    // Verify credentials (db_verify_user handles password hashing internally)
    int user_id;
    char name[MAX_NAME_LENGTH];
    if (db_verify_user(email, password, &user_id, name)) {
        // Set session
        g_session.user_id = user_id;
        g_session.is_logged_in = true;
        strncpy(g_session.email, email, MAX_EMAIL_LENGTH - 1);
        g_session.email[MAX_EMAIL_LENGTH - 1] = '\0';  // Ensure null termination
        strncpy(g_session.name, name, MAX_NAME_LENGTH - 1);
        g_session.name[MAX_NAME_LENGTH - 1] = '\0';  // Ensure null termination
        
        // Disconnect the destroy signal to prevent gtk_main_quit
        g_signal_handlers_disconnect_by_func(lw->window, G_CALLBACK(gtk_main_quit), NULL);
        
        // Close login window
        gtk_widget_destroy(lw->window);
        
        // Show dashboard
        show_dashboard_window();
    } else {
        show_error_dialog(GTK_WINDOW(lw->window), "Invalid email or password.");
    }
}

static void on_register_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    LoginWindow *lw = (LoginWindow *)user_data;
    
    const char *email = gtk_entry_get_text(GTK_ENTRY(lw->reg_email_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(lw->reg_password_entry));
    const char *name = gtk_entry_get_text(GTK_ENTRY(lw->name_entry));
    
    // Validate inputs
    if (!validate_required(name)) {
        show_error_dialog(GTK_WINDOW(lw->window), "Please enter your name.");
        return;
    }
    
    if (!validate_email(email)) {
        show_error_dialog(GTK_WINDOW(lw->window), "Please enter a valid email address.");
        return;
    }
    
    if (!validate_password(password, 8)) {
        show_error_dialog(GTK_WINDOW(lw->window), "Password must be at least 8 characters and contain at least one digit or special character.");
        return;
    }
    
    // Check if user exists
    if (db_user_exists(email)) {
        show_error_dialog(GTK_WINDOW(lw->window), "An account with this email already exists. Please login or use a different email.");
        return;
    }
    
    // Hash password (16 char salt + 64 char hash + null)
    char password_hash[81];
    if (!hash_password(password, password_hash, sizeof(password_hash))) {
        show_error_dialog(GTK_WINDOW(lw->window), "Failed to process password.");
        return;
    }
    
    // Create user
    if (db_create_user(email, password_hash, name)) {
        show_info_dialog(GTK_WINDOW(lw->window), "Account created successfully! You can now login.");
        gtk_stack_set_visible_child(GTK_STACK(lw->main_stack), lw->login_box);
        
        // Clear registration fields
        gtk_entry_set_text(GTK_ENTRY(lw->name_entry), "");
        gtk_entry_set_text(GTK_ENTRY(lw->reg_email_entry), "");
        gtk_entry_set_text(GTK_ENTRY(lw->reg_password_entry), "");
    } else {
        show_error_dialog(GTK_WINDOW(lw->window), "Failed to create account. Please try again.");
    }
}

static void on_show_register_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    LoginWindow *lw = (LoginWindow *)user_data;
    gtk_stack_set_visible_child(GTK_STACK(lw->main_stack), lw->register_box);
}

static void on_show_login_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    LoginWindow *lw = (LoginWindow *)user_data;
    gtk_stack_set_visible_child(GTK_STACK(lw->main_stack), lw->login_box);
}

static GtkWidget* create_login_page(LoginWindow *lw) {
    GtkWidget *page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Header
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 30);
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_ctx, "login-header");
    
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Welcome Back</span>");
    gtk_box_pack_start(GTK_BOX(header_box), title, FALSE, FALSE, 0);
    
    GtkWidget *subtitle = gtk_label_new("Sign in to continue to Aakalan");
    gtk_box_pack_start(GTK_BOX(header_box), subtitle, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(page_box), header_box, FALSE, FALSE, 0);
    
    // Form container
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(form_box), 30);
    
    // Email field
    GtkWidget *email_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(email_label), "<b>Email Address</b>");
    gtk_widget_set_halign(email_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(form_box), email_label, FALSE, FALSE, 0);
    
    lw->login_email_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(lw->login_email_entry), "Enter your email");
    gtk_widget_set_size_request(lw->login_email_entry, -1, 45);
    GtkStyleContext *email_ctx = gtk_widget_get_style_context(lw->login_email_entry);
    gtk_style_context_add_class(email_ctx, "login-input");
    gtk_box_pack_start(GTK_BOX(form_box), lw->login_email_entry, FALSE, FALSE, 0);
    
    // Password field
    GtkWidget *password_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(password_label), "<b>Password</b>");
    gtk_widget_set_halign(password_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(form_box), password_label, FALSE, FALSE, 0);
    
    lw->login_password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(lw->login_password_entry), "Enter your password");
    gtk_entry_set_visibility(GTK_ENTRY(lw->login_password_entry), FALSE);
    gtk_widget_set_size_request(lw->login_password_entry, -1, 45);
    GtkStyleContext *pass_ctx = gtk_widget_get_style_context(lw->login_password_entry);
    gtk_style_context_add_class(pass_ctx, "login-input");
    gtk_box_pack_start(GTK_BOX(form_box), lw->login_password_entry, FALSE, FALSE, 0);
    
    // Login button
    GtkWidget *login_button = gtk_button_new_with_label("Sign In");
    gtk_widget_set_size_request(login_button, -1, 45);
    GtkStyleContext *btn_ctx = gtk_widget_get_style_context(login_button);
    gtk_style_context_add_class(btn_ctx, "login-btn");
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_clicked), lw);
    gtk_box_pack_start(GTK_BOX(form_box), login_button, FALSE, FALSE, 10);
    
    // Register link
    GtkWidget *link_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(link_box, GTK_ALIGN_CENTER);
    
    GtkWidget *link_text = gtk_label_new("Don't have an account?");
    gtk_box_pack_start(GTK_BOX(link_box), link_text, FALSE, FALSE, 0);
    
    GtkWidget *register_link = gtk_button_new_with_label("Register");
    GtkStyleContext *link_ctx = gtk_widget_get_style_context(register_link);
    gtk_style_context_add_class(link_ctx, "link-btn");
    g_signal_connect(register_link, "clicked", G_CALLBACK(on_show_register_clicked), lw);
    gtk_box_pack_start(GTK_BOX(link_box), register_link, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(form_box), link_box, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(page_box), form_box, TRUE, TRUE, 0);
    
    return page_box;
}

static GtkWidget* create_register_page(LoginWindow *lw) {
    GtkWidget *page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Header
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 30);
    GtkStyleContext *header_ctx = gtk_widget_get_style_context(header_box);
    gtk_style_context_add_class(header_ctx, "login-header");
    
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Create Account</span>");
    gtk_box_pack_start(GTK_BOX(header_box), title, FALSE, FALSE, 0);
    
    GtkWidget *subtitle = gtk_label_new("Sign up to get started");
    gtk_box_pack_start(GTK_BOX(header_box), subtitle, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(page_box), header_box, FALSE, FALSE, 0);
    
    // Form container
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(form_box), 30);
    
    // Name field
    GtkWidget *name_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(name_label), "<b>Full Name</b>");
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(form_box), name_label, FALSE, FALSE, 0);
    
    lw->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(lw->name_entry), "Enter your full name");
    gtk_widget_set_size_request(lw->name_entry, -1, 45);
    GtkStyleContext *name_ctx = gtk_widget_get_style_context(lw->name_entry);
    gtk_style_context_add_class(name_ctx, "login-input");
    gtk_box_pack_start(GTK_BOX(form_box), lw->name_entry, FALSE, FALSE, 0);
    
    // Email field
    GtkWidget *email_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(email_label), "<b>Email Address</b>");
    gtk_widget_set_halign(email_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(form_box), email_label, FALSE, FALSE, 0);
    
    lw->reg_email_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(lw->reg_email_entry), "Enter your email");
    gtk_widget_set_size_request(lw->reg_email_entry, -1, 45);
    GtkStyleContext *email_ctx = gtk_widget_get_style_context(lw->reg_email_entry);
    gtk_style_context_add_class(email_ctx, "login-input");
    gtk_box_pack_start(GTK_BOX(form_box), lw->reg_email_entry, FALSE, FALSE, 0);
    
    // Password field
    GtkWidget *password_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(password_label), "<b>Password</b>");
    gtk_widget_set_halign(password_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(form_box), password_label, FALSE, FALSE, 0);
    
    lw->reg_password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(lw->reg_password_entry), "Create a password (min. 6 characters)");
    gtk_entry_set_visibility(GTK_ENTRY(lw->reg_password_entry), FALSE);
    gtk_widget_set_size_request(lw->reg_password_entry, -1, 45);
    GtkStyleContext *pass_ctx = gtk_widget_get_style_context(lw->reg_password_entry);
    gtk_style_context_add_class(pass_ctx, "login-input");
    gtk_box_pack_start(GTK_BOX(form_box), lw->reg_password_entry, FALSE, FALSE, 0);
    
    // Register button
    GtkWidget *register_button = gtk_button_new_with_label("Create Account");
    gtk_widget_set_size_request(register_button, -1, 45);
    GtkStyleContext *btn_ctx = gtk_widget_get_style_context(register_button);
    gtk_style_context_add_class(btn_ctx, "login-btn");
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_clicked), lw);
    gtk_box_pack_start(GTK_BOX(form_box), register_button, FALSE, FALSE, 10);
    
    // Login link
    GtkWidget *link_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(link_box, GTK_ALIGN_CENTER);
    
    GtkWidget *link_text = gtk_label_new("Already have an account?");
    gtk_box_pack_start(GTK_BOX(link_box), link_text, FALSE, FALSE, 0);
    
    GtkWidget *login_link = gtk_button_new_with_label("Sign In");
    GtkStyleContext *link_ctx = gtk_widget_get_style_context(login_link);
    gtk_style_context_add_class(link_ctx, "link-btn");
    g_signal_connect(login_link, "clicked", G_CALLBACK(on_show_login_clicked), lw);
    gtk_box_pack_start(GTK_BOX(link_box), login_link, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(form_box), link_box, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(page_box), form_box, TRUE, TRUE, 0);
    
    return page_box;
}

void show_login_window(void) {
    LoginWindow *lw = g_malloc(sizeof(LoginWindow));
    
    // Apply CSS
    apply_login_css();
    
    // Create window
    lw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(lw->window), "Aakalan");
    gtk_window_set_default_size(GTK_WINDOW(lw->window), 450, 550);
    gtk_window_set_position(GTK_WINDOW(lw->window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(lw->window), FALSE);
    
    // Main container with shadow effect
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(main_container), 20);
    GtkStyleContext *container_ctx = gtk_widget_get_style_context(main_container);
    gtk_style_context_add_class(container_ctx, "login-container");
    
    // Create stack for switching between login and register
    lw->main_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(lw->main_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(lw->main_stack), 300);
    
    // Create pages
    lw->login_box = create_login_page(lw);
    lw->register_box = create_register_page(lw);
    
    // Add pages to stack
    gtk_stack_add_named(GTK_STACK(lw->main_stack), lw->login_box, "login");
    gtk_stack_add_named(GTK_STACK(lw->main_stack), lw->register_box, "register");
    
    gtk_box_pack_start(GTK_BOX(main_container), lw->main_stack, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(lw->window), main_container);
    
    g_signal_connect(lw->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_show_all(lw->window);
}
