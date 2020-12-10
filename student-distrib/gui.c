#include "gui.h"
#include "lib.h"

/* Shape storage: 
 * Maximum of 3 windows, stored inside a window_t array
 * Each window has 2 rectangles and a 3 circle buttons, stored inside 2D rectangle array and circle array. 
 *      First rectangle is the main border of the window.
 *      Second rectangle is the strip occupying the top of the window
 *      Three buttons, RGB, whill be drawn on the top left of the screen inside the second rectangle.
 */
uint16_t rectangle_arr_count = 0;
uint16_t circle_arr_count = 0;
uint16_t window_count=0;
rectangle_t* rectangle_arr[MAX_WINDOW][RECT_NUM];
circle_t* circle_arr[MAX_WINDOW][CIRC_NUM];
window_t* window_arr[MAX_WINDOW];

/*
 * make_rectangle
 *   DESCRIPTION: 	Creates a rectangle using rectangle_t struct
 *   INPUTS:        x	        - position x to draw rect
					y	        - position y to draw rect
					color		- color to use in palette
					width		- x dim of rect
					height		- y dim of rect
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */   
void make_rectangle(uint16_t window_id, uint16_t x, inuint16_tt y, uint16_t color, uint16_t width, uint16_t height){
    
    rectangle_t new_rectangle; 
    new_rectangle->window_id = window_id;
    new_rectangle->id = rectangle_arr_size;
    new_rectangle->x = x;
    new_rectangle->y = y;
    new_rectangle->color = color;
    new_rectangle->width = width;
    new_rectangle->height = height;
    rectangle_arr[window_id][rectangle_arr_size[window_id]] = new_rectangle;
    rectangle_arr_count ++;
    //rectangle_arr_size[window_id]++;
} 

/*
 * make_circle
 *   DESCRIPTION: 	Creates a circle using circle_t struct
 *   INPUTS:        x	        - position x to draw circle
					y	        - position y to draw circle
					color		- color to use in palette
					radius      - radius of circle
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */   
void make_circle(uint16_t window_id, uint16_t x, uint16_t y, uint16_t color, uint16_t radius){
    circle_t new_circle;
    new_circle->window_id = window_id;
    new_circle->id = circle_arr_size;
    new_circle->x = x;
    new_circle->y = y;
    new_circle->color = color;
    new_circle->radius = radius;
    circle_arr_count ++;
    //circle_arr[window_id][circle_arr_size[window_id]] = new_circle;
}


/*
 * make_window
 *   DESCRIPTION: 	Creates a window using window_t struct
 *   INPUTS:        x	        - position x to draw window
					y	        - position y to draw window
					color		- color to use in palette
					width		- x dim of rect
					height		- y dim of rect
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on successful. -1 on already maxed window count
 *   SIDE EFFECTS: none
 */   
int make_window(uint16_t x, uint16_t y, uint16_t color, uint16_t width, uint16_t height){

    // Sanity check: If 3 windows already created, return -1;
    if(window_count>2){
        return -1
    }
    make_rectangle(window_count, x, y, TOGGLE_BAR_COLOR, width, TOGGLE_BAR_HEIGHT); // Create toggle bar
    make_rectangle(window_count, x, y, WINDOW_COLOR, width, height);                // Create main window
    make_circle(window_count, x + BUTTON_OFFSET, y + BUTTON_OFFSET, RED, BUTTON_RAD);                               // Create Red button
    make_circle(window_count, x + BUTTON_OFFSET + BUTTON_GAP, y + BUTTON_OFFSET, YELLOW, BUTTON_RAD);               // Create Yellow button 
    make_circle(window_count, x + BUTTON_OFFSET + BUTTON_GAP + BUTTON_GAP, y + BUTTON_OFFSET, GREEN, BUTTON_RAD);   // Create Green button.
    window_count++;
    // reset rectangle and circle counter for the next window.
    rectangle_arr_count = 0;                                                        
    circle_arr_count = 0;
    return 0;
}

/*
 * draw_rectangle
 *   DESCRIPTION: 	Draws rectangle onto the screen
 *   INPUTS:        rect - rectangle object to be drawn
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots rectangle on screen
 */   

void gui_draw_rectangle(rectangle_t* rect){

	int i, j;
	// put the colors into the build buffer
	for(i = 0; i < rect->width; i++){
		for(j = 0; j < rect->height; j++){
            /* Only plot the pixels inside the screen */
            if((rect->x + rect->width) < SCREEN_X_DIM && (rect->y + rect->height) < SCREEN_Y_DIM){
			    plot_pixel(rect->x + rect->width, rect->y + rect->height, rect->color);
                
            }
		}
	}
	// show_screen();
}

/*
 * draw_circle
 *   DESCRIPTION: 	Draws circle onto the screen
 *   INPUTS:        circle - circle object to be drawn to screen
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots circle on screen
 */   

void gui_draw_circle(circle_t* circle){
	int i, j;

	for(i = 0; i < SCREEN_Y_DIM; i++){
		for(j = 0; j < SCREEN_X_DIM; j++){
			if((j - circle->x) * (j - circle->x) + (i - circle->y) * (i - circle->y) <= (circle->radius * circle->radius)){
				plot_pixel(j, i, circle->color);
			}
		}
	}
	// show_screen();
}

/*
 * draw_window
 *   DESCRIPTION: 	Draws window onto the screen
 *   INPUTS:        Window id
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots window on screen
 */   

void gui_draw_window(uint16_t idx){
    /* Draw all the circle and rectangles in the window */
    int i, j;
    for(i = 0; i< RECT_NUM; i++){
        gui_draw_rectangle(rectangle_arr[idx][i]);
    }
    for(j = 0; j< CIRC_NUM; j++){
        gui_draw_circle(circle_arr[idx][j]);
    }
    //show_screen();
}








;
