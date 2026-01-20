#include "common.h"

// Global session
UserSession g_session = {0};

// Global database connection
sqlite3 *g_db = NULL;

void show_error_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(parent,
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
                                   "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_info_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(parent,
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_OK,
                                   "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

bool show_confirm_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog;
    gint result;
    
    dialog = gtk_message_dialog_new(parent,
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_YES_NO,
                                   "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Confirm");
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    return (result == GTK_RESPONSE_YES);
}
