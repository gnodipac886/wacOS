#ifndef _GUI_H
#define _GUI_H
#include "screen.h"
#include "types.h"
#define MAX_WINDOW          3           // Maximum windows = 3
#define RECT_NUM            2           // 2 Rectangles per window
#define CIRC_NUM            3           // 3 Circles (button) per window

#define RED                 48          // Palette index for Red
#define YELLOW              60          // Palette index for Yellow
#define GREEN               12          // Palette index for Green
#define BUTTON_RAD          4           // Window button radius = 4 pixel
#define BUTTON_OFFSET       6           // Button center = 6 pixels from the top and side of the window
#define BUTTON_GAP          10          // Button center to center  = 10 pixel    
#define TOGGLE_BAR_HEIGHT   12          // Window toggle bar height  = 12 pixels 
#define TOGGLE_BAR_COLOR    21          // Palette index Window toggle bar color
#define WINDOW_COLOR        42          // Palette index Main Window 


typedef struct rectangle{
    uint16_t window_id = -1;
    uint16_t id = -1;
    uint16_t width = 0;         // Dimension of the rectangle
    uint16_t height =0;        
    uint16_t x = 0;             // x location on the screen 
    uint16_t y = 0;             // y location on the screen
    uint16_t color = 0;         // r5g6b5
}rectangle_t;

typedef struct circle{    
    uint16_t window_id = -1;
    uint16_t id = -1;
    uint16_t radius = 0;
    uint16_t x = 0;             //center = x + y*screenwidth
    uint16_t y = 0;
    uint16_t color = 0;         // r5g6b5
}circle_t;

typedef struct window{
    rectangle_t main_window;
    rectangle_t top_section_of_window;
    circle_t button1, button2, button3;
}window_t;

void make_rectangle(uint16_t window_id, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint16_t color); 
void make_circle(uint16_t window_id, uint16_t radius, uint16_t x, uint16_t y, uint16_t color);
int make_window(uint16_t x, uint16_t y, uint16_t color, uint16_t width, uint16_t height);

void gui_draw_rectangle(rectangle_t* rect);
void gui_draw_circle(circle_t* circle);
void gui_draw_window(uint16_t idx);
#endif /* _GUI_H */
