#include "boot_menu.h"
#include "graphic/vbe.h"
#include "drivers/keyboard.h"
#include "io.h"
#include "shell/shell.h"
#include "snake/snake.h"

// Helper function for string length
static int string_length(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// Get keypress without interrupts and clear keyboard buffer
static uint8_t get_keypress(void) {
    uint8_t scancode;
    do {
        scancode = inb(0x60);
        // Wait for key press (scancode without 0x80 bit)
    } while (scancode & 0x80 || scancode == 0);
    
    // Clear any pending keyboard buffer to prevent characters appearing
    while (inb(0x64) & 0x01) {
        inb(0x60); // Read and discard any pending scancodes
    }
    
    return scancode;
}

// Draw menu background with gradient and title
void draw_menu_background(void) {
    // Clear screen with gradient background
    vbe_clear_screen(vbe_rgb(5, 5, 30)); // Dark blue background
    
    // Draw gradient effect
    for (int y = 0; y < 600; y += 2) {
        for (int x = 0; x < 1024; x += 2) {
            uint8_t shade = (x + y) % 20;
            vbe_put_pixel(x, y, vbe_rgb(5 + shade, 5 + shade, 30 + shade));
        }
    }
    
    // Draw header with larger text
    char* title = "MINIOS BOOT MENU";
    int title_x = (1024 - (string_length(title) * 8 * 4)) / 2;
    vbe_draw_string(title_x, 40, title, vbe_rgb(0, 255, 255), 4);
    
    // Draw version below title (larger size)
    char* version = "Version 1.0";
    int version_x = (1024 - (string_length(version) * 8 * 3)) / 2;
    vbe_draw_string(version_x, 100, version, vbe_rgb(150, 200, 255), 3);
    
    // Draw decorative line under version
    for (int x = 150; x < 874; x++) {
        for (int y = 130; y < 136; y++) {
            vbe_put_pixel(x, y, vbe_rgb(100, 200, 255));
        }
    }
}

// Draw static menu elements (icons, boxes, instructions)
void draw_static_elements(void) {
    // Draw shell option box and icon
    int shell_y = 170;
    uint32_t shell_box_color = vbe_rgb(40, 40, 100);
    uint32_t shell_highlight_color = vbe_rgb(60, 60, 120);
    
    // Draw shell selection box
    for (int x = 140; x < 884; x++) {
        for (int y = shell_y - 15; y < shell_y + 90; y++) {
            if (x < 150 || x > 874 || y < shell_y - 5 || y > shell_y + 80) {
                vbe_put_pixel(x, y, shell_highlight_color); // Border
            } else {
                vbe_put_pixel(x, y, shell_box_color); // Main area
            }
        }
    }
    
    // Draw static shell icon (MOVED 25 PIXELS UPWARD)
    int shell_icon_x = 160;
    int shell_icon_y = 170; // Moved up from 202 to 177 (25px upward)
    int shell_icon_width = 140;
    int shell_icon_height = 75;
    
    // Draw terminal window frame
    for (int x = shell_icon_x; x < shell_icon_x + shell_icon_width; x++) {
        for (int y = shell_icon_y; y < shell_icon_y + shell_icon_height; y++) {
            // Window border (much thicker)
            if (x <= shell_icon_x + 3 || x >= shell_icon_x + shell_icon_width - 4 || 
                y <= shell_icon_y + 3 || y >= shell_icon_y + shell_icon_height - 4) {
                vbe_put_pixel(x, y, vbe_rgb(220, 220, 220)); // Light gray border
            }
            // Window background
            else if (x > shell_icon_x && x < shell_icon_x + shell_icon_width - 1 && y > shell_icon_y && y < shell_icon_y + shell_icon_height - 1) {
                vbe_put_pixel(x, y, vbe_rgb(0, 0, 0)); // Black background
            }
        }
    }
    
    // Draw terminal title bar (much larger)
    for (int x = shell_icon_x + 4; x < shell_icon_x + shell_icon_width - 4; x++) {
        for (int y = shell_icon_y + 4; y < shell_icon_y + 18; y++) {
            vbe_put_pixel(x, y, vbe_rgb(70, 70, 180)); // Deeper blue title bar
        }
    }
    
    // Draw title bar text "TERMINAL" (larger)
    int title_start_x = shell_icon_x + 12;
    int title_y = shell_icon_y + 10;
    // Draw "TERMINAL" text using block letters
    for (int i = 0; i < 56; i += 8) {
        for (int x = title_start_x + i; x < title_start_x + i + 6; x++) {
            for (int y = title_y; y < title_y + 8; y++) {
                if ((x == title_start_x + i || x == title_start_x + i + 5) && y > title_y && y < title_y + 7) {
                    vbe_put_pixel(x, y, vbe_rgb(255, 255, 255));
                }
                if (y == title_y + 1 || y == title_y + 6) {
                    vbe_put_pixel(x, y, vbe_rgb(255, 255, 255));
                }
            }
        }
    }
    
    // Draw prompt and command line (much larger)
    for (int x = shell_icon_x + 10; x < shell_icon_x + 25; x++) {
        for (int y = shell_icon_y + 30; y < shell_icon_y + 40; y++) {
            vbe_put_pixel(x, y, vbe_rgb(0, 255, 0)); // Green prompt
        }
    }
    
    // Draw command text (much larger)
    for (int x = shell_icon_x + 30; x < shell_icon_x + 90; x++) {
        for (int y = shell_icon_y + 30; y < shell_icon_y + 40; y++) {
            vbe_put_pixel(x, y, vbe_rgb(255, 255, 255)); // White command text
        }
    }
    
    // Draw output lines (much larger)
    for (int x = shell_icon_x + 10; x < shell_icon_x + 100; x++) {
        for (int y = shell_icon_y + 50; y < shell_icon_y + 60; y++) {
            vbe_put_pixel(x, y, vbe_rgb(120, 120, 255)); // Blue output
        }
    }
    
    for (int x = shell_icon_x + 10; x < shell_icon_x + 80; x++) {
        for (int y = shell_icon_y + 65; y < shell_icon_y + 75; y++) {
            vbe_put_pixel(x, y, vbe_rgb(160, 160, 255)); // Lighter blue output
        }
    }
    
    // Draw snake option box and icon (SNAKE ICON STAYS SAME)
    int snake_y = 300;
    uint32_t snake_box_color = vbe_rgb(40, 40, 100);
    uint32_t snake_highlight_color = vbe_rgb(60, 60, 120);
    
    // Draw snake selection box
    for (int x = 140; x < 884; x++) {
        for (int y = snake_y - 15; y < snake_y + 90; y++) {
            if (x < 150 || x > 874 || y < snake_y - 5 || y > snake_y + 80) {
                vbe_put_pixel(x, y, snake_highlight_color); // Border
            } else {
                vbe_put_pixel(x, y, snake_box_color); // Main area
            }
        }
    }
    
    // Draw static snake game icon (KEEP SAME POSITION)
    int game_x = 160;
    int game_y = 305;  // Snake icon stays at same position
    int game_width = 140;
    int game_height = 70;
    
    // Draw game background with detailed grid
    for (int x = game_x; x < game_x + game_width; x++) {
        for (int y = game_y; y < game_y + game_height; y++) {
            // Create a detailed grid pattern
            if ((x % 3 == 0) || (y % 3 == 0)) {
                vbe_put_pixel(x, y, vbe_rgb(0, 35, 0)); // Darker grid lines
            } else {
                vbe_put_pixel(x, y, vbe_rgb(0, 70, 0)); // Medium green background
            }
        }
    }
    
    // Draw game border (much thicker)
    for (int x = game_x; x < game_x + game_width; x++) {
        for (int y = game_y; y < game_y + game_height; y++) {
            if (x <= game_x + 4 || x >= game_x + game_width - 5 || 
                y <= game_y + 4 || y >= game_y + game_height - 5) {
                vbe_put_pixel(x, y, vbe_rgb(0, 200, 0)); // Bright green border
            }
        }
    }
    
    // Draw snake (KEEP SAME POSITION)
    int snake_head_x = game_x + 25;
    int snake_head_y = game_y + 35;
    
    // Snake head (smaller)
    for (int x = snake_head_x - 6; x < snake_head_x + 6; x++) {
        for (int y = snake_head_y - 6; y < snake_head_y + 6; y++) {
            int dx = x - snake_head_x;
            int dy = y - snake_head_y;
            if (dx*dx + dy*dy <= 36) { // Smaller circle
                // Add shading to head
                if (dx > 1) {
                    vbe_put_pixel(x, y, vbe_rgb(0, 220, 0)); // Darker green
                } else {
                    vbe_put_pixel(x, y, vbe_rgb(0, 255, 0)); // Bright green
                }
            }
        }
    }
    
    // Snake eyes (smaller)
    vbe_put_pixel(snake_head_x + 2, snake_head_y - 1, vbe_rgb(0, 0, 0));
    vbe_put_pixel(snake_head_x + 2, snake_head_y + 1, vbe_rgb(0, 0, 0));
    
    // Snake tongue
    for (int x = snake_head_x + 6; x < snake_head_x + 8; x++) {
        for (int y = snake_head_y - 1; y < snake_head_y + 1; y++) {
            vbe_put_pixel(x, y, vbe_rgb(255, 0, 0)); // Red tongue
        }
    }
    
    // Snake body segments (smaller and more compact)
    int body_segments[][2] = {{20, 35}, {18, 30}, {15, 25}, {12, 20}, {15, 15}, {20, 12}, {28, 15}, {32, 22}, {30, 28}, {25, 33}};
    for (int i = 0; i < 10; i++) {
        int seg_x = game_x + body_segments[i][0];
        int seg_y = game_y + body_segments[i][1];
        
        for (int x = seg_x - 4; x < seg_x + 4; x++) {
            for (int y = seg_y - 4; y < seg_y + 4; y++) {
                int dx = x - seg_x;
                int dy = y - seg_y;
                if (dx*dx + dy*dy <= 16) { // Smaller circles
                    uint32_t color = vbe_rgb(0, 180 - (i * 12), 0); // Smooth gradient green
                    vbe_put_pixel(x, y, color);
                }
            }
        }
    }
    
    // Draw food (smaller apple)
    int food_x = game_x + 100;
    int food_y = game_y + 15;
    
    // Apple body (smaller)
    for (int x = food_x - 6; x < food_x + 6; x++) {
        for (int y = food_y - 6; y < food_y + 6; y++) {
            int dx = x - food_x;
            int dy = y - food_y;
            if (dx*dx + dy*dy <= 36) { // Smaller circle
                // Detailed shading
                if (dx > 2) {
                    vbe_put_pixel(x, y, vbe_rgb(200, 0, 0)); // Darker red on right
                } else if (dx < -2) {
                    vbe_put_pixel(x, y, vbe_rgb(255, 80, 80)); // Brighter red on left
                } else {
                    vbe_put_pixel(x, y, vbe_rgb(255, 0, 0)); // Medium red in middle
                }
            }
        }
    }
    
    // Apple highlight (smaller)
    for (int x = food_x - 3; x < food_x; x++) {
        for (int y = food_y - 3; y < food_y - 1; y++) {
            int dx = x - (food_x - 1);
            int dy = y - (food_y - 2);
            if (dx*dx + dy*dy <= 4) {
                vbe_put_pixel(x, y, vbe_rgb(255, 180, 180)); // Pink highlight
            }
        }
    }
    
    // Apple stem (shorter)
    for (int x = food_x - 1; x < food_x + 1; x++) {
        for (int y = food_y - 9; y < food_y - 6; y++) {
            vbe_put_pixel(x, y, vbe_rgb(100, 60, 20)); // Dark brown stem
        }
    }
    
    // Apple leaf (smaller)
    for (int x = food_x + 1; x < food_x + 4; x++) {
        for (int y = food_y - 8; y < food_y - 5; y++) {
            int dx = x - (food_x + 2);
            int dy = y - (food_y - 6);
            if (dx*dx + dy*dy <= 4) {
                vbe_put_pixel(x, y, vbe_rgb(0, 200, 0)); // Bright green leaf
            }
        }
    }
    

        // Draw instructions below the icons with INCREASED TEXT SIZE
        char* line1 = "Use S for Shell, G for Game";
        char* line2 = "Press ENTER to confirm selection";
        
        int line1_x = (1024 - (string_length(line1) * 8 * 3)) / 2;  // Changed from *2 to *3
        int line2_x = (1024 - (string_length(line2) * 8 * 3)) / 2;  // Changed from *2 to *3
        
        vbe_draw_string(line1_x, 450, line1, vbe_rgb(255, 255, 255), 3);  // Changed scale from 2 to 3
        vbe_draw_string(line2_x, 500, line2, vbe_rgb(255, 255, 255), 3);  // Changed scale from 2 to 3
}

// Draw shell option text with selection highlight
void draw_shell_option(int y_position, int is_selected) {
    uint32_t text_color = is_selected ? vbe_rgb(255, 255, 0) : vbe_rgb(200, 200, 255);
    
    // Draw shell text
    char* shell_title = "COMMAND SHELL";
    int title_x = 320;
    int text_y = y_position + 20;// Vertically centered
    vbe_draw_string(title_x, text_y, shell_title, text_color, 3);
}

// Draw snake option text with selection highlight
void draw_snake_option(int y_position, int is_selected) {
    uint32_t text_color = is_selected ? vbe_rgb(255, 255, 0) : vbe_rgb(200, 200, 255);
    
    // Draw snake text
    char* snake_title = "SNAKE GAME";
    int title_x = 320;
    int text_y = y_position + 50;//ertically centered
    vbe_draw_string(title_x, text_y, snake_title, text_color, 3);
}
    

// Main boot menu function
void show_boot_menu(void) {
    int selected_option = 0; // 0 = shell, 1 = snake
    int menu_active = 1;
    
    // TEMPORARILY DISABLE KEYBOARD INTERRUPTS FOR SHELL
    // This prevents the shell's keyboard handler from processing menu keys
    asm volatile ("cli"); // Disable interrupts
    
    // Clear any pending keyboard buffer
    while (inb(0x64) & 0x01) {
        inb(0x60); // Read and discard any pending scancodes
    }
    
    // Draw static elements only once
    draw_menu_background();
    draw_static_elements();
    
    // Draw initial selection
    draw_shell_option(180, selected_option == 0);
    draw_snake_option(280, selected_option == 1);
    
    while (menu_active) {
        // Handle input using direct port reading (no interrupts)
        uint8_t scancode = get_keypress();
        
        switch (scancode) {
            case 0x1F: // S key - select shell
                if (selected_option != 0) {
                    selected_option = 0;
                    // Only redraw the text (hover colors)
                    draw_shell_option(180, 1);
                    draw_snake_option(280, 0);
                }
                break;
                
            case 0x22: // G key - select snake
                if (selected_option != 1) {
                    selected_option = 1;
                    // Only redraw the text (hover colors)
                    draw_shell_option(180, 0);
                    draw_snake_option(280, 1);
                }
                break;
                
            case 0x1C: // ENTER key - confirm selection
                menu_active = 0;
                // RE-ENABLE INTERRUPTS before launching applications
                asm volatile ("sti");
                if (selected_option == 0) {
                    run_shell_from_menu();
                } else {
                    run_snake_from_menu();
                }
                // When returning, disable interrupts again and redraw
                asm volatile ("cli");
                // Clear any pending keyboard buffer
                while (inb(0x64) & 0x01) {
                    inb(0x60);
                }
                draw_menu_background();
                draw_static_elements();
                draw_shell_option(180, selected_option == 0);
                draw_snake_option(280, selected_option == 1);
                break;
                
            case 0x01: // ESC key - go to shell by default
                menu_active = 0;
                // RE-ENABLE INTERRUPTS before launching applications
                asm volatile ("sti");
                run_shell_from_menu();
                // When returning, disable interrupts again and redraw
                asm volatile ("cli");
                // Clear any pending keyboard buffer
                while (inb(0x64) & 0x01) {
                    inb(0x60);
                }
                draw_menu_background();
                draw_static_elements();
                draw_shell_option(180, 1);
                draw_snake_option(280, 0);
                break;
        }
    }
    
    // RE-ENABLE INTERRUPTS when leaving menu
    asm volatile ("sti");
}

// Launch shell from menu
void run_shell_from_menu(void) {
    // Clear screen
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    // Initialize shell
    shell_init();
    
    // Wait for exit command
    while (!shell_should_exit()) {
        // The shell runs via keyboard interrupts
        // Just wait here until exit command is executed
        for (volatile int i = 0; i < 10000; i++);
    }
    
    // Clear screen and return to menu
    vbe_clear_screen(vbe_rgb(10, 10, 40));
}

// Launch snake game from menu
void run_snake_from_menu(void) {
    // Clear screen and start snake game normally (with menu)
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    // Start snake game normally - when it finishes, it will return here
    snake_game();  // Changed from snake_game_direct() to snake_game()
    
    // Clear screen and return to menu
    vbe_clear_screen(vbe_rgb(10, 10, 40));
}