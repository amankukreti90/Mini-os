#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define MAX_COMMAND_LENGTH 256
#define MARGIN_LEFT 20
#define MARGIN_TOP 50
#define LINE_HEIGHT 20
#define CHAR_WIDTH 8

typedef struct {
    char username[32];
    char hostname[32];
    char current_path[256];
    int cursor_x;
    int cursor_y;
} shell_t;

// Shell functions
void shell_init(void);                              // Initialize shell environment
void shell_clear_screen(void);                      // Clear shell display area
void shell_draw_prompt(void);                       // Draw command prompt
void shell_add_char(char c);                        // Add character to command line
void shell_execute_command(const char *command);    // Execute shell command
void shell_print(const char *text, uint32_t color); // Print text to shell
void shell_new_line(void);                          // Move to new line
void shell_change_directory(const char *path);      // Change current directory

// Screen functions
int shell_get_screen_height(void);                  // Get available screen height
void shell_scroll_screen(void);                     // Scroll screen content

// Exit functions - ADD THESE
int shell_should_exit(void);                        // Check if shell should exit
void shell_reset_exit(void);                        // Reset exit flag

extern shell_t shell;  // Global shell instance

#endif