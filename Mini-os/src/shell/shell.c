#include "shell.h"
#include "commands.h"
#include "../graphic/vbe.h"
#include "../fs/filesystem.h"
#include "../drivers/keyboard.h"
#include "../drivers/rtc.h"

// Global shell instance
shell_t shell;

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_pos = 0;
static int output_cursor_x = 5;
static int output_cursor_y = 5;

// Forward declarations
void shell_new_line(void);
int shell_get_screen_height(void);
void shell_scroll_screen(void);
void shell_print(const char *text, uint32_t color);

// Basic string utilities
static void shell_strcpy(char *dest, const char *src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

static int shell_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static int shell_strlen(const char *str) {
    int len = 0; while (*str++) len++; return len;
}

static int str_equal(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) { s1++; s2++; }
    return (*s1 == '\0' && *s2 == '\0');
}

// Move to next line with automatic scroll
void shell_new_line(void) {
    output_cursor_x = 5;
    output_cursor_y += LINE_HEIGHT;
    if (output_cursor_y + LINE_HEIGHT > shell_get_screen_height()) {
        shell_scroll_screen();
    }
}

// Get display dimensions from VBE
int shell_get_screen_height(void) {
    extern struct vbe_mode_info* vbe_info;
    return (vbe_info && vbe_info->framebuffer) ? vbe_info->height : 600;
}

static int shell_get_screen_width(void) {
    extern struct vbe_mode_info* vbe_info;
    return (vbe_info && vbe_info->framebuffer) ? vbe_info->width : 800;
}

// Clear entire screen and reset shell state
void shell_clear_screen(void) {
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    output_cursor_x = 5;
    output_cursor_y = 5;
    command_pos = 0;
    command_buffer[0] = '\0';
}

// Scroll screen content up by one line
void shell_scroll_screen(void) {
    extern struct vbe_mode_info* vbe_info;
    if (!vbe_info || !vbe_info->framebuffer) return;
    
    int screen_width = vbe_info->width;
    int screen_height = vbe_info->height;
    int pitch = vbe_info->pitch;
    int bpp = vbe_info->bpp / 8;
    uint8_t* fb = (uint8_t*)vbe_info->framebuffer;
    
    // Copy pixel data up by one line
    for (int y = 5; y < screen_height - LINE_HEIGHT; y++) {
        for (int x = 0; x < screen_width; x++) {
            uint8_t* src_pixel = fb + ((y + LINE_HEIGHT) * pitch) + (x * bpp);
            uint8_t* dest_pixel = fb + (y * pitch) + (x * bpp);
            
            if (bpp == 2) {
                *(uint16_t*)dest_pixel = *(uint16_t*)src_pixel;
            } else if (bpp == 3) {
                dest_pixel[0] = src_pixel[0]; dest_pixel[1] = src_pixel[1]; dest_pixel[2] = src_pixel[2];
            } else if (bpp == 4) {
                *(uint32_t*)dest_pixel = *(uint32_t*)src_pixel;
            }
        }
    }
    
    // Clear the new bottom line
    for (int y = screen_height - LINE_HEIGHT; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            vbe_put_pixel(x, y, vbe_rgb(0, 0, 0));
        }
    }
    output_cursor_y -= LINE_HEIGHT;
}

// Output text at current cursor position with color
void shell_print(const char *text, uint32_t color) {
    const char *ptr = text;
    while (*ptr) {
        if (*ptr == '\n') {
            output_cursor_x = 5;
            output_cursor_y += LINE_HEIGHT;
            if (output_cursor_y + LINE_HEIGHT > shell_get_screen_height()) {
                shell_scroll_screen();
            }
        } else {
            vbe_draw_char(output_cursor_x, output_cursor_y, *ptr, color, 2);
            output_cursor_x += CHAR_WIDTH * 2;
        }
        ptr++;
    }
}

// Display shell prompt
void shell_draw_prompt(void) {
    char prompt[128];
    int pos = 0;
    
    const char *user = shell.username;
    while (*user) prompt[pos++] = *user++;
    prompt[pos++] = '@';
    
    const char *host = shell.hostname;
    while (*host) prompt[pos++] = *host++;
    prompt[pos++] = ':';
    
    const char *path = shell.current_path;
    while (*path) prompt[pos++] = *path++;
    prompt[pos++] = '$'; prompt[pos++] = ' '; prompt[pos] = '\0';
    
    shell_print(prompt, vbe_rgb(0, 255, 0));
}

// Initialize shell with default settings
void shell_init(void) {
    shell_strcpy(shell.username, "mini");
    shell_strcpy(shell.hostname, "miniOS");
    shell_strcpy(shell.current_path, "~");
    
    command_pos = 0;
    command_buffer[0] = '\0';
    output_cursor_x = 5;
    output_cursor_y = 5;
    
    shell_clear_screen();
    shell_draw_prompt();
    
    int prompt_length = shell_strlen("mini@miniOS:~$ ");
    output_cursor_x = 5 + (prompt_length * CHAR_WIDTH * 2);
    shell.cursor_x = output_cursor_x;
    shell.cursor_y = output_cursor_y;
}

// Change current working directory
void shell_change_directory(const char *path) {
    if (shell_strcmp(path, "/") == 0 || shell_strcmp(path, "~") == 0) {
        shell_strcpy(shell.current_path, "~");
    } else if (shell_strcmp(path, "..") == 0) {
        shell_strcpy(shell.current_path, "~");
    } else {
        shell_strcpy(shell.current_path, path);
    }
}

// Handle keyboard input character by character
void shell_add_char(char c) {
    if (c == '\b') { // Backspace
        if (command_pos > 0) {
            // Clear character from screen
            for (int y = output_cursor_y; y < output_cursor_y + 20; y++) {
                for (int x = output_cursor_x - (CHAR_WIDTH * 2); x < output_cursor_x; x++) {
                    vbe_put_pixel(x, y, vbe_rgb(0, 0, 0));
                }
            }
            command_pos--;
            command_buffer[command_pos] = '\0';
            output_cursor_x -= CHAR_WIDTH * 2;
            
            // Handle line wrapping
            if (output_cursor_x < 5) {
                int screen_width = shell_get_screen_width();
                output_cursor_x = screen_width - (CHAR_WIDTH * 2);
                output_cursor_y -= LINE_HEIGHT;
            }
            shell.cursor_x = output_cursor_x;
            shell.cursor_y = output_cursor_y;
        }
    } else if (c == '\n') { // Execute command
        output_cursor_x = 5;
        output_cursor_y += LINE_HEIGHT;
        if (output_cursor_y + LINE_HEIGHT > shell_get_screen_height()) {
            shell_scroll_screen();
        }
        
        shell_execute_command(command_buffer);
        
        command_pos = 0;
        command_buffer[0] = '\0';
        output_cursor_x = 5;
        output_cursor_y += LINE_HEIGHT;
        if (output_cursor_y + LINE_HEIGHT > shell_get_screen_height()) {
            shell_scroll_screen();
        }
        
        shell_draw_prompt();
        int prompt_length = shell_strlen("mini@miniOS:~$ ");
        output_cursor_x = 5 + (prompt_length * CHAR_WIDTH * 2);
        shell.cursor_x = output_cursor_x;
        shell.cursor_y = output_cursor_y;
    } else if (command_pos < MAX_COMMAND_LENGTH - 1) { // Normal character
        vbe_draw_char(output_cursor_x, output_cursor_y, c, vbe_rgb(255, 255, 255), 2);
        command_buffer[command_pos++] = c;
        command_buffer[command_pos] = '\0';
        output_cursor_x += CHAR_WIDTH * 2;
        
        // Handle line wrapping
        int screen_width = shell_get_screen_width();
        if (output_cursor_x + (CHAR_WIDTH * 2) > screen_width) {
            output_cursor_x = 5;
            output_cursor_y += LINE_HEIGHT;
            if (output_cursor_y + LINE_HEIGHT > shell_get_screen_height()) {
                shell_scroll_screen();
            }
        }
        shell.cursor_x = output_cursor_x;
        shell.cursor_y = output_cursor_y;
    }
}

// Parse and execute shell commands
void shell_execute_command(const char *command) {
    if (shell_strlen(command) == 0) return;
    
    const char *ptr = command;
    int i = 0;
    char parsed_cmd_name[32] = {0};
    char parsed_cmd_arg1[256] = {0};
    char parsed_cmd_arg2[1024] = {0};
    
    // Extract command name
    while (*ptr && *ptr != ' ' && i < 31) {
        parsed_cmd_name[i++] = *ptr++;
    }
    parsed_cmd_name[i] = '\0';
    
    // Special handling for echo command with quotes
    if (str_equal(parsed_cmd_name, "echo")) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '"') {
            ptr++; i = 0;
            while (*ptr && *ptr != '"' && i < 1023) {
                parsed_cmd_arg1[i++] = *ptr++;
            }
            parsed_cmd_arg1[i] = '\0';
            if (*ptr == '"') ptr++;
        } else {
            i = 0;
            while (*ptr && i < 1023) {
                parsed_cmd_arg1[i++] = *ptr++;
            }
            parsed_cmd_arg1[i] = '\0';
        }
    } else {
        // Standard parsing for other commands
        while (*ptr == ' ') ptr++;
        i = 0;
        while (*ptr && *ptr != ' ' && i < 255) {
            parsed_cmd_arg1[i++] = *ptr++;
        }
        parsed_cmd_arg1[i] = '\0';
        
        while (*ptr == ' ') ptr++;
        i = 0;
        while (*ptr && i < 1023) {
            parsed_cmd_arg2[i++] = *ptr++;
        }
        parsed_cmd_arg2[i] = '\0';
    }
    
    // Copy to global variables for command handlers
    extern char cmd_name[32], cmd_arg1[256], cmd_arg2[1024];
    shell_strcpy(cmd_name, parsed_cmd_name);
    shell_strcpy(cmd_arg1, parsed_cmd_arg1);
    shell_strcpy(cmd_arg2, parsed_cmd_arg2);
    
    // Dispatch to command handlers
    if (str_equal(parsed_cmd_name, "help") || str_equal(parsed_cmd_name, "?")) {
        cmd_help();
    } else if (str_equal(parsed_cmd_name, "clear") || str_equal(parsed_cmd_name, "cls")) {
        cmd_clear();
    } else if (str_equal(parsed_cmd_name, "ls") || str_equal(parsed_cmd_name, "dir")) {
        cmd_ls();
    } else if (str_equal(parsed_cmd_name, "cd")) {
        cmd_cd();
    } else if (str_equal(parsed_cmd_name, "pwd")) {
        cmd_pwd();
    } else if (str_equal(parsed_cmd_name, "create") || str_equal(parsed_cmd_name, "touch")) {
        cmd_create();
    } else if (str_equal(parsed_cmd_name, "write")) {
        cmd_write();
    } else if (str_equal(parsed_cmd_name, "read") || str_equal(parsed_cmd_name, "cat")) {
        cmd_read();
    } else if (str_equal(parsed_cmd_name, "delete") || str_equal(parsed_cmd_name, "rm")) {
        cmd_delete();
    } else if (str_equal(parsed_cmd_name, "whoami")) {
        cmd_whoami();
    } else if (str_equal(parsed_cmd_name, "hostname")) {
        cmd_hostname();
    } else if (str_equal(parsed_cmd_name, "date")) {
        cmd_date();
    } else if (str_equal(parsed_cmd_name, "uname")) {
        cmd_uname();
    } else if (str_equal(parsed_cmd_name, "echo")) {
        cmd_echo();
    } else if (str_equal(parsed_cmd_name, "exit") || str_equal(parsed_cmd_name, "logout")) {
        cmd_exit();
    } else if (shell_strlen(parsed_cmd_name) > 0) {
        shell_print("Command not found: ", vbe_rgb(255, 0, 0));
        shell_print(parsed_cmd_name, vbe_rgb(255, 255, 255));
        shell_print("\n", vbe_rgb(255, 255, 255));
        shell_print("Type 'help' for available commands\n", vbe_rgb(255, 255, 0));
    }
}