#ifndef _OCTREE_H
#define _OCTREE_H

#include "types.h"
#include "screen.h"

#define LOW1BITS 			0x0001
#define LOW2BITS 			0x0003
#define LOW4BITS 			0x000F

/*
	An octree object. It holds the information such as occurange for the histogram,
	index for that color in the palette, as well as the r, g, b, values and means.
*/
typedef struct octree{
	int occur;
	int pal_idx;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint32_t r_mean;
    uint32_t g_mean;
    uint32_t b_mean;
} octree_t;

void __init_octree__();
uint8_t* get_palette(pixel_565_t* photo);
void update_octree(pixel_565_t pixel);
int compare_octree(const void * a, const void * b);
void create_palette192();

#endif /* _OCTREE_H */
