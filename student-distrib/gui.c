#include "gui.h"
#include "screen.h"
#include "lib.h"

/* Shape storage: 
 * Maximum of 3 windows, stored inside a window_t array
 * Each window has 2 rectangles and a 3 circle buttons, stored inside 2D rectangle array and circle array. 
 *      First rectangle is the main border of the window.
 *      Second rectangle is the strip occupying the top of the window
 *      Three buttons, RGB, whill be drawn on the top left of the screen inside the second rectangle.
 */
int rectangle_arr_count = 0;
int circle_arr_count = 0;
int window_count = 0;
rectangle_t rectangle_arr[MAX_WINDOW][RECT_NUM];
circle_t circle_arr[MAX_WINDOW][CIRC_NUM];
window_t window_arr[MAX_WINDOW];

void __init_gui__(){
    make_window(80, 40, 160, 120);
	save_window_background(window_count - 1);

    gui_draw_window(0);
    save_cursor_background();
}

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
void make_rectangle(int window_id, int width, int height, int x, int y, int color){
    
    rectangle_t new_rectangle; 
    new_rectangle.window_id = window_id;
    new_rectangle.id = rectangle_arr_count;
    new_rectangle.x = x;
    new_rectangle.y = y;
    new_rectangle.color = color;
    new_rectangle.width = width;
    new_rectangle.height = height;
    rectangle_arr[window_id][rectangle_arr_count] = new_rectangle;
    rectangle_arr_count++;
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
void make_circle(int window_id, int radius, int x, int y, int color){
    circle_t new_circle;
    new_circle.window_id = window_id;
    new_circle.id = circle_arr_count;
    new_circle.x = x;
    new_circle.y = y;
    new_circle.color = color;
    new_circle.radius = radius;
    circle_arr[window_id][circle_arr_count] = new_circle;
    circle_arr_count ++;
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
int make_window(int x, int y, int width, int height){

    // Sanity check: If 3 windows already created, return -1;
    if(window_count>2){
        return -1;
    }
    make_rectangle(window_count, width, height, x, y, WINDOW_COLOR);                // Create main window
    make_rectangle(window_count, width, TOGGLE_BAR_HEIGHT, x, y, TOGGLE_BAR_COLOR); // Create toggle bar
    make_circle(window_count, BUTTON_RAD, x + BUTTON_OFFSET, y + BUTTON_OFFSET, RED);                               // Create Red button
    make_circle(window_count, BUTTON_RAD, x + BUTTON_OFFSET + BUTTON_GAP, y + BUTTON_OFFSET, YELLOW);               // Create Yellow button 
    make_circle(window_count, BUTTON_RAD, x + BUTTON_OFFSET + BUTTON_GAP + BUTTON_GAP, y + BUTTON_OFFSET, GREEN);   // Create Green button.
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

void gui_draw_rectangle(rectangle_t rect){

	int i, j;
	// put the colors into the build buffer
	for(i = 0; i < rect.width; i++){
		for(j = 0; j < rect.height; j++){
            /* Only plot the pixels inside the screen */
            if((rect.x + i) < SCREEN_X_DIM 
                && (rect.y + j) < SCREEN_Y_DIM
                && (rect.x + i) > 0
                && (rect.y + j) > 0){
			    plot_pixel(rect.x + i, rect.y + j, rect.color);
                
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

void gui_draw_circle(circle_t circle){
	int i, j;

	for(i = 0; i < SCREEN_Y_DIM; i++){
		for(j = 0; j < SCREEN_X_DIM; j++){
            if((j - circle.x) * (j - circle.x) + (i - circle.y) * (i - circle.y) <= (circle.radius * circle.radius)){
				plot_pixel(j, i, circle.color);
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

void gui_draw_window(int idx){
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

/*
 * change_window_location
 *   DESCRIPTION: 	If mouse is dragging window, changes the location of the window
 *   INPUTS:        Window id, dx, dy
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots window on screen
 */
void change_window_location(int curr_x, int curr_y, int dx, int dy, int frames, int sx, int sy){
    int idx, i, save_x, save_y;
	frames = frames == 0 ? 1 : frames;
	frames = frames / 2 == 0 ? 1 : frames / 2;
    for(i = 0; i < frames; i++){
        for(idx = 0; idx < window_count; idx++){
            /* Check if mouse is dragging the window */
            if(curr_x < rectangle_arr[idx][1].x + rectangle_arr[idx][1].width 
            && curr_x > rectangle_arr[idx][1].x 
            && curr_y < rectangle_arr[idx][1].y + rectangle_arr[idx][1].height 
            && curr_y > rectangle_arr[idx][1].y ){
                
                // write original screen
                for(save_y = 0; save_y < rectangle_arr[idx][0].height; save_y++){
                    for(save_x = 0; save_x < rectangle_arr[idx][0].width; save_x++){
                        plot_pixel(rectangle_arr[idx][0].x + save_x, rectangle_arr[idx][0].y + save_y, window_arr[idx].window_background_buf[save_y * rectangle_arr[idx][0].width + save_x]);
                    }
                }

                for(i = 0; i < RECT_NUM; i++){
                    rectangle_arr[idx][i].x += dx / frames == 0 ? sx : dx / frames;
                    rectangle_arr[idx][i].y -= dy / frames == 0 ? sy : dy / frames;
                }

                for(i = 0; i < CIRC_NUM; i++){
                    circle_arr[idx][i].x += dx / frames == 0 ? sx : dx / frames;
                    circle_arr[idx][i].y -= dy / frames == 0 ? sy : dy / frames;
                }

                //for (i = 0; i < MAX_WINDOW; i++){
                save_window_background(idx);
                //}
            }
            gui_draw_window(idx);
        }
		if (!(dx / frames) && !(dy / frames)){
			break;
		}
	}
}

/*
 * save_window_background
 *   DESCRIPTION: 	none
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: saves background of a window
 */
void save_window_background(int idx){
	int save_x, save_y;
	// save the middle mouse frame
	for(save_y = 0; save_y < rectangle_arr[idx][0].height; save_y++){
		for(save_x = 0; save_x < rectangle_arr[idx][0].width; save_x++){
			window_arr[idx].window_background_buf[save_y * rectangle_arr[idx][0].width + save_x] = get_pixel(rectangle_arr[idx][0].x + save_x, rectangle_arr[idx][0].y + save_y);
		}
	}
}



