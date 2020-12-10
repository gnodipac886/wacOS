#ifndef _GUI_H
#define _GUI_H

#include "types.h"

#define MAX_WINDOW          3           // Maximum windows = 3
#define RECT_NUM            2           // 2 Rectangles per window
#define CIRC_NUM            3           // 3 Circles (button) per window

#define RED                 48          // Palette index for Red
#define YELLOW              60          // Palette index for Yellow
#define GREEN               12          // Palette index for Green
#define BUTTON_RAD          3           // Window button radius = 4 pixel
#define BUTTON_OFFSET       6           // Button center = 6 pixels from the top and side of the window
#define BUTTON_GAP          10          // Button center to center  = 10 pixel    
#define TOGGLE_BAR_HEIGHT   12          // Window toggle bar height  = 12 pixels 
#define TOGGLE_BAR_COLOR    21          // Palette index Window toggle bar color
#define WINDOW_COLOR        42          // Palette index Main Window 
#define WINDOW_WIDTH        160         // Default window width = 160 pixels
#define WINDOW_HEIGHT       120         // Default window height = 120 pixels

typedef struct rectangle{
    int window_id;
    int id;
    int width;         // Dimension of the rectangle
    int height;        
    int x;             // x location on the screen 
    int y;             // y location on the screen
    int color;         // r5g6b5
}rectangle_t;

typedef struct circle{    
    int window_id;
    int id;
    int radius;
    int x;             //center = x + y * screenwidth
    int y;
    int color;         // r5g6b5
}circle_t;

typedef struct window{
    rectangle_t main_window;
    rectangle_t top_section_of_window;
    circle_t button1, button2, button3;
	uint8_t window_background_buf[WINDOW_WIDTH * WINDOW_HEIGHT];
}window_t;

void make_rectangle(int window_id, int width, int height, int x, int y, int color); 
void make_circle(int window_id, int radius, int x, int y, int color);
int make_window(int x, int y, int width, int height);

void __init_gui__();
void gui_draw_rectangle(rectangle_t rect);
void gui_draw_circle(circle_t circle);
void gui_draw_window(int idx);
void change_window_location(int curr_x, int curr_y, int dx, int dy, int frames, int sx, int sy);
void save_window_background(int idx);
#endif /* _GUI_H */
