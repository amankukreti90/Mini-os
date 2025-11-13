#ifndef BOOT_MENU_H
#define BOOT_MENU_H

#include "graphic/vbe.h"
#include "drivers/keyboard.h"
#include "io.h"
#include "shell/shell.h"
#include "snake/snake.h"

// Set current operating mode (menu, shell, game, etc.)
void set_current_mode(int mode);

// Get current operating mode
int get_current_mode(void);

// Show main boot menu with application selection
void show_boot_menu(void);

// Launch shell application from menu
void run_shell_from_menu(void);

// Launch snake game from menu
void run_snake_from_menu(void);

#endif