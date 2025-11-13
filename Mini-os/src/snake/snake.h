#ifndef SNAKE_H
#define SNAKE_H

#include "../graphic/vbe.h"
#include "../drivers/keyboard.h"
#include "../io.h"

#define SNAKE_MAX_LENGTH 100
#define GRID_SIZE 16
#define GAME_WIDTH 50
#define GAME_HEIGHT 40
#define GAME_OFFSET_X 100
#define GAME_OFFSET_Y 80

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point body[SNAKE_MAX_LENGTH];
    int length;
    int direction;
    int score;
} Snake;

// Game functions
void snake_game(void);
void snake_game_direct(void);  // Direct entry without menu
void snake_init(Snake *snake);
void generate_food(Point *food, Snake *snake);
int check_collision(Snake *snake);
void draw_game(Snake *snake, Point *food);
int handle_snake_input(void);
void snake_draw_rect(int x, int y, uint32_t color);
void update_snake(Snake *snake);
void show_start_screen(void);

#endif