#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global entry widget
GtkWidget *entry;

// Forward declaration for evaluation function
double eval(const char *expr);

// Function called when any button is clicked
void on_button_clicked(GtkWidget *widget, gpointer data) {
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    const char *current = gtk_entry_get_text(GTK_ENTRY(entry));
    char buffer[256];

    // Clear the entire entry
    if (strcmp(label, "C") == 0) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");

    // Backspace: delete one character
    } else if (strcmp(label, "Del") == 0) {
        int len = strlen(current);
        if (len > 0) {
            strncpy(buffer, current, len - 1);
            buffer[len - 1] = '\0';
            gtk_entry_set_text(GTK_ENTRY(entry), buffer);
        }

    // Evaluate the expression
    } else if (strcmp(label, "=") == 0) {
        double result = eval(current);
        sprintf(buffer, "%.10g", result);
        gtk_entry_set_text(GTK_ENTRY(entry), buffer);

    // Wrap current expression in function like sin(), cos(), etc.
    } else if (strcmp(label, "sin") == 0 || strcmp(label, "cos") == 0 ||
               strcmp(label, "tan") == 0 || strcmp(label, "log") == 0 ||
               strcmp(label, "ln") == 0 || strcmp(label, "√") == 0) {
        snprintf(buffer, sizeof(buffer), "%s(%s)", label, current);
        gtk_entry_set_text(GTK_ENTRY(entry), buffer);

    // Append number/operator to the entry
    } else {
        snprintf(buffer, sizeof(buffer), "%s%s", current, label);
        gtk_entry_set_text(GTK_ENTRY(entry), buffer);
    }
}

// Simple expression evaluator
double eval(const char *expr) {
    char op;
    double num1, num2;
    char func[10];

    // Handle single-argument functions like sin(0.5)
    if (sscanf(expr, "%[a-z](%lf)", func, &num1) == 2) {
        if (strcmp(func, "sin") == 0) return sin(num1);
        if (strcmp(func, "cos") == 0) return cos(num1);
        if (strcmp(func, "tan") == 0) return tan(num1);
        if (strcmp(func, "log") == 0) return log10(num1);
        if (strcmp(func, "ln") == 0) return log(num1);
        if (strcmp(func, "sqrt") == 0) return sqrt(num1);
    }

    // Handle basic binary operators: + - * / ^
    if (sscanf(expr, "%lf%c%lf", &num1, &op, &num2) == 3) {
        switch(op) {
            case '+': return num1 + num2;
            case '-': return num1 - num2;
            case '*': return num1 * num2;
            case '/': return num2 != 0 ? num1 / num2 : NAN;
            case '^': return pow(num1, num2);
        }
    }

    // If it's just a number
    return atof(expr);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scientific Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create grid container
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // Create entry field
    entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 5, 1);
    gtk_widget_set_margin_top(entry, 10);
    gtk_widget_set_margin_bottom(entry, 10);
    gtk_widget_set_margin_start(entry, 10);
    gtk_widget_set_margin_end(entry, 10);
    gtk_widget_set_size_request(entry, 300, 50);

    // Button labels
    const char *buttons[6][5] = {
        {"7","8","9","/","√"},
        {"4","5","6","*","^"},
        {"1","2","3","-","log"},
        {"0",".","=","+","ln"},
        {"C","Del","sin","cos","tan"},
        {"(",")","","",""}
    };

    // Create buttons and attach to grid
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 5; j++) {
            if (buttons[i][j][0] != '\0') {
                GtkWidget *button = gtk_button_new_with_label(buttons[i][j]);
                gtk_widget_set_size_request(button, 60, 50);
                gtk_widget_set_margin_top(button, 2);
                gtk_widget_set_margin_bottom(button, 2);
                gtk_widget_set_margin_start(button, 2);
                gtk_widget_set_margin_end(button, 2);
                g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
                gtk_grid_attach(GTK_GRID(grid), button, j, i+1, 1, 1);
            }
        }
    }

    // Show all widgets
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

