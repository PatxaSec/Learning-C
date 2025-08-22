#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global entry widget
GtkWidget *entry;

// Utility: trim spaces
void trim_copy(const char *src, char *dst, size_t n) {
    while (isspace((unsigned char)*src)) src++;
    size_t len = strlen(src);
    while (len > 0 && isspace((unsigned char)src[len-1])) len--;
    if (len >= n) len = n-1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

// Recursive evaluator
double eval(const char *expr) {
    if (!expr) return NAN;

    char s[512];
    trim_copy(expr, s, sizeof(s));
    if (s[0] == '\0') return 0.0;

    // Parentheses
    if (s[0] == '(') {
        int depth = 0;
        size_t i;
        for (i = 0; s[i]; i++) {
            if (s[i] == '(') depth++;
            else if (s[i] == ')') {
                depth--;
                if (depth == 0 && s[i+1] == '\0') {
                    char inner[512];
                    size_t inner_len = i - 1;
                    if (inner_len >= sizeof(inner)) inner_len = sizeof(inner)-1;
                    memcpy(inner, s+1, inner_len);
                    inner[inner_len] = '\0';
                    return eval(inner);
                }
            }
        }
    }

    double num1;
    char func[32];

    // √number  (symbol)
    if ((unsigned char)s[0] == 0xE2) { // UTF-8 "√" starts with 0xE2
        return sqrt(eval(s+3)); // skip 3-byte UTF-8 symbol
    }

    // sqrt(...)
    if (sscanf(s, "sqrt(%lf)", &num1) == 1) return sqrt(num1);

    // sin, cos, tan, log, ln
    if (sscanf(s, "%31[a-z](%lf)", func, &num1) == 2) {
        if (strcmp(func,"sin")==0) return sin(num1);
        if (strcmp(func,"cos")==0) return cos(num1);
        if (strcmp(func,"tan")==0) return tan(num1);
        if (strcmp(func,"log")==0) return log10(num1);
        if (strcmp(func,"ln")==0)  return log(num1);
    }

    // Operator precedence: +-, then */, then ^
    const char *groups[] = { "+-", "*/", "^" };
    for (int g = 0; g < 3; g++) {
        int depth = 0;
        for (ssize_t i = (ssize_t)strlen(s)-1; i >= 0; i--) {
            char c = s[i];
            if (c == ')') { depth++; continue; }
            if (c == '(') { depth--; continue; }
            if (depth != 0) continue;

            if (strchr(groups[g], c)) {
                // unary +/- detection
                if ((c == '+' || c == '-')) {
                    ssize_t j = i-1;
                    while (j >= 0 && isspace((unsigned char)s[j])) j--;
                    if (j < 0) continue;
                    if (strchr("+-*/^(", s[j])) continue;
                }

                // split
                char left[512], right[512];
                strncpy(left, s, i);
                left[i] = '\0';
                strncpy(right, s+i+1, sizeof(right)-1);
                right[sizeof(right)-1] = '\0';

                double L = eval(left);
                double R = eval(right);

                switch(c) {
                    case '+': return L + R;
                    case '-': return L - R;
                    case '*': return L * R;
                    case '/': return (R != 0.0) ? L / R : NAN;
                    case '^': return pow(L, R);
                }
            }
        }
    }

    // Fallback: number
    char *endptr = NULL;
    double val = strtod(s, &endptr);
    if (endptr && *endptr == '\0') return val;

    return NAN;
}

// Button callback
void on_button_clicked(GtkWidget *widget, gpointer data) {
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    const char *current = gtk_entry_get_text(GTK_ENTRY(entry));
    char buffer[512];

    if (strcmp(label, "C") == 0) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    } else if (strcmp(label, "Del") == 0) {
        int len = strlen(current);
        if (len > 0) {
            strncpy(buffer, current, len - 1);
            buffer[len - 1] = '\0';
            gtk_entry_set_text(GTK_ENTRY(entry), buffer);
        }
    } else if (strcmp(label, "=") == 0) {
        double result = eval(current);
        if (isnan(result))
            gtk_entry_set_text(GTK_ENTRY(entry), "Error");
        else {
            sprintf(buffer, "%.10g", result);
            gtk_entry_set_text(GTK_ENTRY(entry), buffer);
        }
    } else if (strcmp(label, "sin") == 0 || strcmp(label, "cos") == 0 ||
               strcmp(label, "tan") == 0 || strcmp(label, "log") == 0 ||
               strcmp(label, "ln") == 0) {
        snprintf(buffer, sizeof(buffer), "%s(%s)", label, current);
        gtk_entry_set_text(GTK_ENTRY(entry), buffer);
    } else if (strcmp(label, "√") == 0) {
        snprintf(buffer, sizeof(buffer), "√%s", current);
        gtk_entry_set_text(GTK_ENTRY(entry), buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "%s%s", current, label);
        gtk_entry_set_text(GTK_ENTRY(entry), buffer);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scientific Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 5, 1);
    gtk_widget_set_margin_top(entry, 10);
    gtk_widget_set_margin_bottom(entry, 10);
    gtk_widget_set_margin_start(entry, 10);
    gtk_widget_set_margin_end(entry, 10);
    gtk_widget_set_size_request(entry, 300, 50);

    const char *buttons[6][5] = {
        {"7","8","9","/","√"},
        {"4","5","6","*","^"},
        {"1","2","3","-","log"},
        {"0",".","=","+","ln"},
        {"C","Del","sin","cos","tan"},
        {"(",")","","",""}
    };

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

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
