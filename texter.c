#include <gtk/gtk.h>
#include <string.h>

// Function to open a file
static void open_file(GtkWidget *widget, gpointer text_view) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar *content;
        gsize length;
        if (g_file_get_contents(filename, &content, &length, NULL)) {
            gtk_text_buffer_set_text(buffer, content, length);
            g_free(content);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Function to save a file
static void save_file(GtkWidget *widget, gpointer text_view) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        g_file_set_contents(filename, text, -1, NULL);
        g_free(filename);
    }

    g_free(text);
    gtk_widget_destroy(dialog);
}

// Function to find text
static void find_text(GtkWidget *widget, gpointer user_data) {
    GtkTextView *text_view = GTK_TEXT_VIEW(user_data);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Find Text",
                                                    NULL,
                                                    GTK_DIALOG_MODAL,
                                                    "_Find", GTK_RESPONSE_ACCEPT,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *entry = gtk_entry_new();
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(entry));
        GtkTextIter start, match_start, match_end;
        gtk_text_buffer_get_start_iter(buffer, &start);

        gboolean found = gtk_text_iter_forward_search(&start,
                                                      search_text,
                                                      GTK_TEXT_SEARCH_TEXT_ONLY,
                                                      &match_start,
                                                      &match_end,
                                                      NULL);
        if (found) {
            gtk_text_buffer_select_range(buffer, &match_start, &match_end);
            gtk_text_view_scroll_to_iter(text_view, &match_start, 0.0, FALSE, 0, 0);
        } else {
            GtkWidget *msg = gtk_message_dialog_new(NULL,
                                                    GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Text not found.");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        }
    }

    gtk_widget_destroy(dialog);
}

// Cut function
static void cut_text(GtkMenuItem *menuitem, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

// Copy function
static void copy_text(GtkMenuItem *menuitem, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

// Paste function
static void paste_text(GtkMenuItem *menuitem, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Mini Text Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Menu
    GtkWidget *menu_bar = gtk_menu_bar_new();
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *edit_menu = gtk_menu_new();

    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Edit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);

    GtkWidget *open_item = gtk_menu_item_new_with_label("Open");
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);

    GtkWidget *cut_item = gtk_menu_item_new_with_label("Cut");
    GtkWidget *copy_item = gtk_menu_item_new_with_label("Copy");
    GtkWidget *paste_item = gtk_menu_item_new_with_label("Paste");
    GtkWidget *find_item = gtk_menu_item_new_with_label("Find");
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), cut_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), copy_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), paste_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), find_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_item);
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Text view
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Connect menu actions
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    g_signal_connect(open_item, "activate", G_CALLBACK(open_file), text_view);
    g_signal_connect(save_item, "activate", G_CALLBACK(save_file), text_view);
    g_signal_connect(cut_item, "activate", G_CALLBACK(cut_text), buffer);
    g_signal_connect(copy_item, "activate", G_CALLBACK(copy_text), buffer);
    g_signal_connect(paste_item, "activate", G_CALLBACK(paste_text), buffer);
    g_signal_connect(find_item, "activate", G_CALLBACK(find_text), text_view);
    if (argc > 1) {
        gchar *content;
        gsize length;
        if (g_file_get_contents(argv[1], &content, &length, NULL)) {
            gtk_text_buffer_set_text(buffer, content, length);
            g_free(content);
        } else {
            g_print("No se pudo abrir el archivo: %s\n", argv[1]);
        }
    }

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
