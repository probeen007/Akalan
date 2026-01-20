#ifndef UI_CLASSES_H
#define UI_CLASSES_H

#include <gtk/gtk.h>
#include "common.h"

void show_create_class_window(GtkWindow *parent, void (*on_class_created_callback)(void));

#endif // UI_CLASSES_H
