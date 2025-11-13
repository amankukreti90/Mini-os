#ifndef MOUSE_H
#define MOUSE_H

void init_mouse(void);
void handle_mouse(void);
void mouse_set_screen_bounds(int width, int height);
void erase_mouse_cursor(void);
void draw_mouse_cursor(void);
int get_mouse_x(void);
int get_mouse_y(void);
int get_mouse_buttons(void);

#endif