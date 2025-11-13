#ifndef COMMANDS_H
#define COMMANDS_H

#include "../graphic/vbe.h"
#include "../fs/filesystem.h"
#include "../drivers/rtc.h"

// Command function declarations
void cmd_help(void);      // Display available commands
void cmd_clear(void);     // Clear screen
void cmd_ls(void);        // List files
void cmd_cd(void);        // Change directory
void cmd_pwd(void);       // Print working directory
void cmd_create(void);    // Create file
void cmd_write(void);     // Write to file
void cmd_read(void);      // Read file content
void cmd_delete(void);    // Delete file
void cmd_whoami(void);    // Display current user
void cmd_hostname(void);  // Display system hostname
void cmd_date(void);      // Show current date/time
void cmd_uname(void);     // Display system information
void cmd_echo(void);      // Echo text to screen
void cmd_exit(void);      // Exit shell

#endif