#include "snake.h"
#include "../graphic/vbe.h"
#include "../drivers/keyboard.h"
#include "../io.h"
#include "../boot_menu.h" 

// Global game state
static Snake snake;
static Point food;
static int game_running;
static int last_direction;

// Game area boundaries for optimized drawing
#define GAME_AREA_START_X 1
#define GAME_AREA_END_X (GAME_WIDTH - 2)
#define GAME_AREA_START_Y 1
#define GAME_AREA_END_Y (GAME_HEIGHT - 2)

// Helper function for string length
static int string_length(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// Get keypress without interrupts
static uint8_t get_keypress(void) {
    uint8_t scancode;
    do {
        scancode = inb(0x60);
    } while (scancode & 0x80 || scancode == 0);
    return scancode;
}

// Show snake game menu (before game starts)
void show_snake_start_menu(void) {
    // DISABLE INTERRUPTS to prevent shell from processing keys
    asm volatile ("cli");
    
    // Clear any pending keyboard buffer
    while (inb(0x64) & 0x01) {
        inb(0x60);
    }
    
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    char* title = "SNAKE GAME";
    char* menu_items[] = {
        "P - PLAY GAME",
        "Q - QUIT TO MAIN MENU"
    };
    
    char* controls[] = {
        "CONTROLS:",
        "ARROW KEYS - MOVE SNAKE"
    };
    
    char* start_prompt = "PRESS P TO PLAY OR Q TO QUIT";
    
    // Draw title (larger size)
    int title_x = (1024 - (string_length(title) * 8 * 4)) / 2;
    vbe_draw_string(title_x, 120, title, vbe_rgb(0, 255, 0), 4);
    
    // Draw menu items (larger size and reduced gap)
    for (int i = 0; i < 2; i++) {
        int text_x = (1024 - (string_length(menu_items[i]) * 8 * 3)) / 2;
        vbe_draw_string(text_x, 220 + (i * 50), menu_items[i], vbe_rgb(0, 255, 0), 3);
    }
    
    // Draw controls (larger size and reduced gap)
    for (int i = 0; i < 2; i++) {
        int text_x = (1024 - (string_length(controls[i]) * 8 * 2)) / 2;
        vbe_draw_string(text_x, 340 + (i * 35), controls[i], vbe_rgb(255, 255, 255), 2);
    }
    
    // Draw start prompt (larger size)
    int prompt_x = (1024 - (string_length(start_prompt) * 8 * 2)) / 2;
    vbe_draw_string(prompt_x, 430, start_prompt, vbe_rgb(255, 255, 0), 2);
    
    // Wait for specific key press (P or Q only)
    while (1) {
        uint8_t scancode = get_keypress();
        
        // Clear keyboard buffer
        while (inb(0x64) & 0x01) {
            inb(0x60);
        }
        
        if (scancode == 0x10) { // Q key - quit
            game_running = 0;
            break;
        } else if (scancode == 0x19) { // P key - play game
            game_running = 1;
            break;
        }
        // Ignore all other keys
    }
    
    // RE-ENABLE INTERRUPTS before returning
    asm volatile ("sti");
}

// Show game over screen with final score
void show_game_over_screen(void) {
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    char game_over[] = "GAME OVER";
    char final_score[32];
    int pos = 0;
    
    // Build final score string
    char score_text[] = "FINAL SCORE: ";
    for (int i = 0; score_text[i]; i++) {
        final_score[pos++] = score_text[i];
    }
    
    int score = snake.score;
    int digit1 = (score / 100) % 10;
    int digit2 = (score / 10) % 10;
    int digit3 = score % 10;
    
    final_score[pos++] = '0' + digit1;
    final_score[pos++] = '0' + digit2;
    final_score[pos++] = '0' + digit3;
    final_score[pos] = '\0';
    
    char prompt[] = "PRESS ENTER TO CONTINUE";
    
    // Calculate text widths for proper centering
    int game_over_width = string_length(game_over) * 8 * 4;  // Scale 4
    int score_width = string_length(final_score) * 8 * 4;    // Scale 4  
    int prompt_width = string_length(prompt) * 8 * 3;        // Scale 3
    
    // Draw GAME OVER - shifted slightly right and properly centered
    int game_over_x = (1024 - game_over_width) / 2 + 8;  // +8 pixels to shift right
    vbe_draw_string(game_over_x, 220, game_over, vbe_rgb(255, 0, 0), 4);
    
    // Draw FINAL SCORE - properly centered
    int score_x = (1024 - score_width) / 2;
    vbe_draw_string(score_x, 280, final_score, vbe_rgb(255, 255, 0), 4);
    
    // Draw prompt - properly centered  
    int prompt_x = (1024 - prompt_width) / 2;
    vbe_draw_string(prompt_x, 340, prompt, vbe_rgb(255, 255, 255), 3);
    
    // Wait for Enter key to return to boot menu
    while (1) {
        uint8_t scancode = get_keypress();
        if (scancode == 0x1C) { // Enter key
            break;
        }
    }
}    

// Initialize snake with starting position and direction
void snake_init(Snake *snake) {
    // Initialize snake in the middle of the game area
    snake->length = 3;
    snake->direction = 1; // Start moving right
    snake->score = 0;
    last_direction = 1;
    
    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = GAME_WIDTH / 2 - i;
        snake->body[i].y = GAME_HEIGHT / 2;
    }
}

// Generate food at random position not overlapping with snake
void generate_food(Point *food, Snake *snake) {
    int valid_position = 0;
    static int seed = 0;
    
    while (!valid_position) {
        // Simple pseudo-random using a counter
        seed++;
        food->x = GAME_AREA_START_X + (seed % (GAME_AREA_END_X - GAME_AREA_START_X));
        food->y = GAME_AREA_START_Y + ((seed * 7) % (GAME_AREA_END_Y - GAME_AREA_START_Y));
        
        valid_position = 1;
        // Check if food overlaps with snake
        for (int i = 0; i < snake->length; i++) {
            if (snake->body[i].x == food->x && snake->body[i].y == food->y) {
                valid_position = 0;
                break;
            }
        }
    }
}

// Check for wall collision or self collision
int check_collision(Snake *snake) {
    Point head = snake->body[0];
    
    // Wall collision - matches the border boundaries
    if (head.x < GAME_AREA_START_X || head.x > GAME_AREA_END_X || 
        head.y < GAME_AREA_START_Y || head.y > GAME_AREA_END_Y) {
        return 1;
    }
    
    // Self collision (skip first segment)
    for (int i = 1; i < snake->length; i++) {
        if (head.x == snake->body[i].x && head.y == snake->body[i].y) {
            return 1;
        }
    }
    
    return 0;
}

// Draw a single grid cell at position (x,y)
void snake_draw_rect(int x, int y, uint32_t color) {
    int pixel_x = GAME_OFFSET_X + x * GRID_SIZE;
    int pixel_y = GAME_OFFSET_Y + y * GRID_SIZE;
    
    // Draw filled rectangle
    for (int py = 0; py < GRID_SIZE - 1; py++) {
        for (int px = 0; px < GRID_SIZE - 1; px++) {
            vbe_put_pixel(pixel_x + px, pixel_y + py, color);
        }
    }
}

// Draw border that matches the collision boundaries
void draw_border(void) {
    uint32_t border_color = vbe_rgb(255, 255, 0); // Solid yellow
    
    // Draw border around the actual game area (where collision happens)
    for (int x = GAME_AREA_START_X - 1; x <= GAME_AREA_END_X + 1; x++) {
        snake_draw_rect(x, GAME_AREA_START_Y - 1, border_color);
        snake_draw_rect(x, GAME_AREA_END_Y + 1, border_color);
    }
    for (int y = GAME_AREA_START_Y - 1; y <= GAME_AREA_END_Y + 1; y++) {
        snake_draw_rect(GAME_AREA_START_X - 1, y, border_color);
        snake_draw_rect(GAME_AREA_END_X + 1, y, border_color);
    }
}

// Draw the entire snake properly with head and body gradient
void draw_snake_complete(Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        uint32_t color;
        if (i == 0) {
            color = vbe_rgb(0, 255, 0); // Head - bright green
        } else {
            // Body with gradient - darker as we go further from head
            int green = 200 - (i * 15);
            if (green < 50) green = 50;
            color = vbe_rgb(0, green, 0);
        }
        snake_draw_rect(snake->body[i].x, snake->body[i].y, color);
    }
}

// Draw food as red apple
void draw_food(Point *food) {
    // Draw food as a single red apple
    uint32_t apple_color = vbe_rgb(255, 0, 0); // Solid red
    
    // Draw main apple body only
    snake_draw_rect(food->x, food->y, apple_color);
}

// Draw score display above game area
void draw_score(Snake *snake) {
    // Draw score with larger text and fixed width
    char score_text[32];
    int score = snake->score;
    
    // Always show 3 digits with leading zeros
    int digit1 = (score / 100) % 10;
    int digit2 = (score / 10) % 10;
    int digit3 = score % 10;
    
    // Build score string with fixed width
    char final_text[] = "SCORE: ";
    int final_len = 0;
    
    // Copy "SCORE: "
    for (int i = 0; final_text[i]; i++) {
        score_text[final_len++] = final_text[i];
    }
    
    // Add leading zeros and score digits
    score_text[final_len++] = '0' + digit1;
    score_text[final_len++] = '0' + digit2;
    score_text[final_len++] = '0' + digit3;
    score_text[final_len] = '\0';
    
    // Clear the score area first to avoid artifacts
    int score_bg_x = GAME_OFFSET_X;
    int score_bg_y = GAME_OFFSET_Y - 45;
    int score_bg_width = 200;
    int score_bg_height = 30;
    
    for (int y = score_bg_y; y < score_bg_y + score_bg_height; y++) {
        for (int x = score_bg_x; x < score_bg_x + score_bg_width; x++) {
            vbe_put_pixel(x, y, vbe_rgb(0, 0, 0));
        }
    }
    
    // Draw score with larger font size (scale 2)
    vbe_draw_string(GAME_OFFSET_X, GAME_OFFSET_Y - 40, score_text, vbe_rgb(255, 255, 255), 2);
}

// Optimized game drawing - food never blinks
void draw_game_frame(Snake *snake, Point *food) {
    // Clear only the game area (not the border or food)
    for (int y = GAME_AREA_START_Y; y <= GAME_AREA_END_Y; y++) {
        for (int x = GAME_AREA_START_X; x <= GAME_AREA_END_X; x++) {
            // Check if this position is not food
            if (x != food->x || y != food->y) {
                snake_draw_rect(x, y, vbe_rgb(0, 0, 0));
            }
        }
    }
    
    // Draw the complete snake
    draw_snake_complete(snake);
    
    // Food is always drawn and never cleared, so no blinking
    draw_food(food);
    
    // Update score
    draw_score(snake);
}

// Initial game drawing setup
void draw_game_initial(Snake *snake, Point *food) {
    // Clear the entire game area only once at start
    for (int y = GAME_AREA_START_Y; y <= GAME_AREA_END_Y; y++) {
        for (int x = GAME_AREA_START_X; x <= GAME_AREA_END_X; x++) {
            snake_draw_rect(x, y, vbe_rgb(0, 0, 0));
        }
    }
    
    // Draw initial snake
    draw_snake_complete(snake);
    
    // Draw initial food
    draw_food(food);
    
    // Draw initial score
    draw_score(snake);
}

// Handle keyboard input for snake movement
int handle_snake_input(void) {
    uint8_t scancode = inb(0x60);
    
    // REMOVED: No quit during gameplay
    // Only handle arrow keys
    
    // Handle arrow keys (PS/2 scancodes)
    if (scancode == 0x48) { // Up arrow
        if (last_direction != 2) snake.direction = 0;
    } else if (scancode == 0x4D) { // Right arrow
        if (last_direction != 3) snake.direction = 1;
    } else if (scancode == 0x50) { // Down arrow
        if (last_direction != 0) snake.direction = 2;
    } else if (scancode == 0x4B) { // Left arrow
        if (last_direction != 1) snake.direction = 3;
    }
    
    last_direction = snake.direction;
    return 1;
}

// Update snake position by moving body segments
void update_snake(Snake *snake) {
    // Move snake body (from tail to head)
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    
    // Move head based on direction
    switch (snake->direction) {
        case 0: // Up
            snake->body[0].y--;
            break;
        case 1: // Right
            snake->body[0].x++;
            break;
        case 2: // Down
            snake->body[0].y++;
            break;
        case 3: // Left
            snake->body[0].x--;
            break;
    }
}

// Grow snake when eating food
void grow_snake(Snake *snake) {
    if (snake->length < SNAKE_MAX_LENGTH) {
        // The new tail segment should be at the same position as the current tail
        snake->body[snake->length] = snake->body[snake->length - 1];
        snake->length++;
    }
}

// Simple delay function for game timing
void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++);
}

// Main snake game function
void snake_game(void) {
    // Show start menu BEFORE game starts
    show_snake_start_menu();
    
    if (!game_running) {
        return; // Return to boot menu if Q was pressed
    }
    
    // RESET GAME STATE
    game_running = 1;
    last_direction = 1;
    
    // Initialize game
    snake_init(&snake);
    generate_food(&food, &snake);
    
    // Clear screen for game
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    // Draw border once (it stays throughout the game)
    draw_border();
    
    // Draw initial game state
    draw_game_initial(&snake, &food);
    
    // Game timing variables - SLOWER SPEED
    int frame_count = 0;
    int game_speed = 80; // Higher = slower movement
    
    // Main game loop
    while (game_running) {
        // Handle input every frame
        game_running = handle_snake_input();
        
        // Update game state at controlled speed
        if (frame_count % game_speed == 0) {
            update_snake(&snake);
            
            // Check collisions
            if (check_collision(&snake)) {
                break;
            }
            
            // Check food collision
            if (snake.body[0].x == food.x && snake.body[0].y == food.y) {
                // Properly grow snake
                grow_snake(&snake);
                snake.score += 10;
                
                // Increase speed slightly as score increases
                if (snake.score % 100 == 0 && game_speed > 50) {
                    game_speed--;
                }
                
                generate_food(&food, &snake);
            }
            
            // Redraw game frame (food never blinks)
            draw_game_frame(&snake, &food);
        }
        
        frame_count++;
        delay(200000); // Slow speed delay
    }
    
    // Show simple game over screen and return to boot menu
    show_game_over_screen();
    // Function returns automatically to boot menu
}