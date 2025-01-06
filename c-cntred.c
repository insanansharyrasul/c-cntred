#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef struct termios termios;
typedef struct winsize winsize;

void get_terminal_size(int *cols, int *rows) {
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    *cols = ws.ws_col;
    *rows = ws.ws_row;
}

termios enable_raw_mode() {
    termios original, raw;
    tcgetattr(STDIN_FILENO, &original);
    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    return original;
}

void disable_raw_mode(termios original) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

void enable_alternate_screen() {
    printf("\x1B[?1049h");
    fflush(stdout);
}

void disable_alternate_screen() {
    printf("\x1B[?1049l");
    fflush(stdout);
}

void clear_screen() {
    printf("\x1B[2J\x1B[H");
    fflush(stdout);
}

void print_centered(const char *text) {
    int cols, rows;
    get_terminal_size(&cols, &rows);

    int x = (cols - strlen(text)) / 2;
    int y = rows / 2;

    printf("\x1B[%d;%dH%s", y, x, text);
    fflush(stdout);
}

void wait_for_key_exit(char exit_key) {
    char buffer;
    while (1) {
        if (read(STDIN_FILENO, &buffer, 1) > 0) {
            if (buffer == exit_key) {
                break;
            }
        }
        usleep(10000);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <text_to_center>\n", argv[0]);
        return 1;
    }

    size_t total_length = 0;
    for (int i = 1; i < argc; i++) {
        total_length += strlen(argv[i]) + 1;
    }

    char *text = malloc(total_length);
    if (!text) {
        perror("malloc");
        return 1;
    }

    text[0] = '\0';
    for (int i = 1; i < argc; i++) {
        strcat(text, argv[i]);
        if (i < argc - 1) {
            strcat(text, " ");
        }
    }

    termios original = enable_raw_mode();

    enable_alternate_screen();

    clear_screen();
    print_centered(text);

    int cols, rows;
    get_terminal_size(&cols, &rows);
    printf("\x1B[%d;1HPress 'Ctrl-C' or 'q' to quit.", rows);
    fflush(stdout);

    wait_for_key_exit('q');

    disable_alternate_screen();
    disable_raw_mode(original);

    free(text);

    return 0;
}
