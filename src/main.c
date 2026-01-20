#include "common.h"
#include "database.h"
#include "ui_login.h"
#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Initialize database
    const char *db_path = "data/assignment_tracker.db";
    if (!db_init(db_path)) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }
    
    printf("Assignment Tracker System\n");
    printf("Database initialized at: %s\n", db_path);
    
    // Show login window
    show_login_window();
    
    // Start GTK main loop
    gtk_main();
    
    // Cleanup
    db_cleanup();
    
    return 0;
}
