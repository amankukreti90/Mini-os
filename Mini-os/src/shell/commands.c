#include "commands.h"
#include "../graphic/vbe.h"
#include "../fs/filesystem.h"
#include "../drivers/rtc.h"
#include "shell.h"

// Global command variables
char cmd_name[32] = {0};
char cmd_arg1[256] = {0};
char cmd_arg2[1024] = {0};

// Simple exit flag
static int should_exit = 0;

// Check if shell should exit
int shell_should_exit(void) {
    return should_exit;
}

// Reset exit flag
void shell_reset_exit(void) {
    should_exit = 0;
}

// Forward declarations
void shell_print(const char *text, uint32_t color);
void shell_new_line(void);
void shell_change_directory(const char *path);
extern shell_t shell;

// String comparison helper
static int str_equal(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) { s1++; s2++; }
    return (*s1 == '\0' && *s2 == '\0');
}

// Display available commands
void cmd_help(void) {
    shell_print("Available commands:\n", vbe_rgb(255, 255, 0));
    shell_print("  help, clear, ls, cd, pwd, create, write, read, echo\n", vbe_rgb(255, 255, 0));
    shell_print("  delete, whoami, hostname, date, uname, exit\n", vbe_rgb(255, 255, 0));
}

// Clear shell screen
void cmd_clear(void) {
    shell_clear_screen();
}

// List files in filesystem
void cmd_ls(void) {
    char file_list[512];
    fs_list_files(file_list);
    shell_print(file_list, vbe_rgb(255, 255, 255));
    shell_print("\n", vbe_rgb(255, 255, 255));
}

// Change current directory
void cmd_cd(void) {
    if (cmd_arg1[0] != '\0') {
        shell_change_directory(cmd_arg1);
        shell_print("Directory changed\n", vbe_rgb(0, 255, 0));
    } else {
        shell_change_directory("~");
        shell_print("Changed to home directory\n", vbe_rgb(0, 255, 0));
    }
}

// Print current working directory
void cmd_pwd(void) {
    shell_print(shell.current_path, vbe_rgb(255, 255, 255));
    shell_print("\n", vbe_rgb(255, 255, 255));
}

// Create new file
void cmd_create(void) {
    if (cmd_arg1[0] != '\0') {
        if (fs_create_file(cmd_arg1) >= 0) {
            shell_print("File created: ", vbe_rgb(0, 255, 0));
            shell_print(cmd_arg1, vbe_rgb(255, 255, 255));
            shell_print("\n", vbe_rgb(255, 255, 255));
        } else {
            shell_print("Error creating file\n", vbe_rgb(255, 0, 0));
        }
    } else {
        shell_print("Usage: create <filename>\n", vbe_rgb(255, 0, 0));
    }
}

// Write content to file
void cmd_write(void) {
    if (cmd_arg1[0] != '\0' && cmd_arg2[0] != '\0') {
        if (fs_write_file(cmd_arg1, cmd_arg2) >= 0) {
            shell_print("Content written to: ", vbe_rgb(0, 255, 0));
            shell_print(cmd_arg1, vbe_rgb(255, 255, 255));
            shell_print("\n", vbe_rgb(255, 255, 255));
        } else {
            shell_print("Error writing to file\n", vbe_rgb(255, 0, 0));
        }
    } else {
        shell_print("Usage: write <filename> <content>\n", vbe_rgb(255, 0, 0));
    }
}

// Read file content
void cmd_read(void) {
    if (cmd_arg1[0] != '\0') {
        char content[1024];
        int result = fs_read_file(cmd_arg1, content);
        if (result >= 0) {
            shell_new_line();
            shell_print("File content: ", vbe_rgb(0, 255, 255));
            shell_new_line();
            shell_print(content, vbe_rgb(255, 255, 255));
            shell_new_line();
        } else {
            shell_print("File not found: ", vbe_rgb(255, 0, 0));
            shell_print(cmd_arg1, vbe_rgb(255, 255, 255));
            shell_print("\n", vbe_rgb(255, 255, 255));
        }
    } else {
        shell_print("Usage: read <filename>\n", vbe_rgb(255, 0, 0));
    }
}

// Delete file
void cmd_delete(void) {
    if (cmd_arg1[0] != '\0') {
        if (fs_delete_file(cmd_arg1) >= 0) {
            shell_print("File deleted: ", vbe_rgb(0, 255, 0));
            shell_print(cmd_arg1, vbe_rgb(255, 255, 255));
            shell_print("\n", vbe_rgb(255, 255, 255));
        } else {
            shell_print("File not found: ", vbe_rgb(255, 0, 0));
            shell_print(cmd_arg1, vbe_rgb(255, 255, 255));
            shell_print("\n", vbe_rgb(255, 255, 255));
        }
    } else {
        shell_print("Usage: delete <filename>\n", vbe_rgb(255, 0, 0));
    }
}

// Display current username
void cmd_whoami(void) {
    shell_print(shell.username, vbe_rgb(255, 255, 255));
    shell_print("\n", vbe_rgb(255, 255, 255));
}

// Display system hostname
void cmd_hostname(void) {
    shell_print(shell.hostname, vbe_rgb(255, 255, 255));
    shell_print("\n", vbe_rgb(255, 255, 255));
}

// Display current date and time
void cmd_date(void) {
    datetime_t current_time;
    rtc_get_time(&current_time);
    
    char date_str[64];
    int pos = 0;
    
    // Format: HH:MM:SS DD/MM/YYYY
    int hour = current_time.hour;
    if (hour < 10) date_str[pos++] = '0';
    date_str[pos++] = '0' + (hour / 10);
    date_str[pos++] = '0' + (hour % 10);
    date_str[pos++] = ':';
    
    int minute = current_time.minute;
    if (minute < 10) date_str[pos++] = '0';
    date_str[pos++] = '0' + (minute / 10);
    date_str[pos++] = '0' + (minute % 10);
    date_str[pos++] = ':';
    
    int second = current_time.second;
    if (second < 10) date_str[pos++] = '0';
    date_str[pos++] = '0' + (second / 10);
    date_str[pos++] = '0' + (second % 10);
    date_str[pos++] = ' ';
    
    int day = current_time.day;
    if (day < 10) date_str[pos++] = '0';
    date_str[pos++] = '0' + (day / 10);
    date_str[pos++] = '0' + (day % 10);
    date_str[pos++] = '/';
    
    int month = current_time.month;
    if (month < 10) date_str[pos++] = '0';
    date_str[pos++] = '0' + (month / 10);
    date_str[pos++] = '0' + (month % 10);
    date_str[pos++] = '/';
    
    int year = current_time.year;
    date_str[pos++] = '0' + (year / 1000);
    date_str[pos++] = '0' + ((year % 1000) / 100);
    date_str[pos++] = '0' + ((year % 100) / 10);
    date_str[pos++] = '0' + (year % 10);
    date_str[pos] = '\0';
    
    shell_print(date_str, vbe_rgb(255, 255, 255));
    shell_print("\n", vbe_rgb(255, 255, 255));
}

// Display system information
void cmd_uname(void) {
    if (cmd_arg1[0] != '\0' && str_equal(cmd_arg1, "-a")) {
        shell_print("miniOS 1.0 x86\n", vbe_rgb(255, 255, 255));
    } else {
        shell_print("miniOS\n", vbe_rgb(255, 255, 255));
    }
}

// Echo text to screen
void cmd_echo(void) {
    shell_print(cmd_arg1, vbe_rgb(255, 255, 255));
    shell_print("\n", vbe_rgb(255, 255, 255));
}

// Exit shell and return to boot menu
void cmd_exit(void) {
    shell_print("Returning to boot menu...\n", vbe_rgb(255, 255, 0));
    
    // Short delay to show message
    for (volatile int i = 0; i < 1000000; i++);
    
    // Set exit flag
    should_exit = 1;
}