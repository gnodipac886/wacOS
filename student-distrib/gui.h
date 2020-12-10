#ifndef _GUI_H
#define _GUI_H

#include "types.h"

typedef struct rectangle{
    uint16_t width;         // Dimension of the rectangle
    uint16_t height;        
    uint16_t x;             // x location on the screen 
    uint16_t y;             // y location on the screen
    uint16_t color;         // r5g6b5
}rectangle_t;

typedef struct circle{
    uint16_t radius;
    uint16_t x;             //center = x + y*screenwidth
    uint16_t y;
    uint16_t color;         // r5g6b5
}circle_t;

typedef struct window{
    rectangle_t main_window;
    rectangle_t top_section_of_window;
    circle_t button1, button2, button3;
}window_t;

int make_rectangle(int x, int y, uint16_t color, int width, int height); 
int make_circle(int x, int y, uint16_t color, int radius);
int make_window(int x, int y, uint16_t color, int width, int height);
#endif /* _GUI_H */
