#include "octree.h"
#include "lib.h"

/*
	the two octree arrays for level 4 and level 2
*/
static octree_t lvl_4[4096];
static octree_t lvl_2[64];
static uint8_t palette[192 * 3];

/*
 * init_octree
 *   DESCRIPTION: initialize the octree
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */ 
void __init_octree__(){
	uint32_t i; 								// counter
	for(i = 0; i < 4096; i++){
		if(i < 64){								// initialize level 2 octree
			lvl_2[i].occur = 0;
			lvl_2[i].pal_idx = -1;
			lvl_2[i].r = (i >> 4) & LOW2BITS;
			lvl_2[i].g = (i >> 2) & LOW2BITS;
			lvl_2[i].b = i & LOW2BITS;
			lvl_2[i].r_mean = 0;
			lvl_2[i].g_mean = 0;
			lvl_2[i].b_mean = 0;
		}
		// initialize level 4 octree
		lvl_4[i].occur = 0;
		lvl_4[i].pal_idx = -1;
		lvl_4[i].r = (i >> 8) & LOW4BITS;
		lvl_4[i].g = (i >> 4) & LOW4BITS;
		lvl_4[i].b = i & LOW4BITS;
		lvl_4[i].r_mean = 0;
		lvl_4[i].g_mean = 0;
		lvl_4[i].b_mean = 0;
	}
}

uint8_t* get_palette(pixel_565_t* photo){
	int i, j;
	uint16_t temp_pic[SCREEN_X_DIM * SCREEN_Y_DIM];

	__init_octree__();

	for(i = 0; i < SCREEN_Y_DIM; i++){
		for(j = 0; j < SCREEN_X_DIM; j++){
			pixel_565_t pixel = photo[SCREEN_X_DIM * i + j];

			temp_pic[SCREEN_X_DIM * i + j] = (((pixel.r >> 1) << 8) | ((pixel.g >> 2) << 4) | (pixel.b >> 1));
			update_octree(pixel);
		}
	}

	create_palette192();

	for(i = 0; i < SCREEN_Y_DIM; i++){
		for(j = 0; j < SCREEN_X_DIM; j++){
			// get the 12 bit pixel
    		uint16_t pix12 = temp_pic[SCREEN_X_DIM * i + j];

    		// check if the index is valid in the level 4 octree
    		if(lvl_4[pix12].pal_idx != -1){
    			*(uint16_t*)&(photo[SCREEN_X_DIM * i + j]) = (uint16_t)(lvl_4[pix12].pal_idx);
    		}
    		else{
    			// find the level 2 octree array index
    			uint8_t pix6 = (((pix12 >> 10) << 4) | (((pix12 >> 6) & 0x3) << 2) | ((pix12 >> 2) & 0x3));

    			// add the palette index into the image data
    			*(uint16_t*)&(photo[SCREEN_X_DIM * i + j]) = (uint16_t)(lvl_2[pix6].pal_idx);
    		}
		}
	}
	return palette;
}

/*
 * update_octree
 *   DESCRIPTION: update the octree with a new pixel
 *   INPUTS: pixel - pixel value
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */ 
void update_octree(pixel_565_t pixel){
	uint8_t r = pixel.r;					// 5 bit red
	uint8_t g = pixel.g;					// 6 bit green
	uint8_t b = pixel.b;					// 5 bit blue

	uint8_t r_up4 = r >> 1;					// upper 4 bits of r
	uint8_t g_up4 = g >> 2;					// upper 4 bits of g
	uint8_t b_up4 = b >> 1;					// upper 4 bits of b

	uint8_t r_up2 = r_up4 >> 2;				// upper 2 bits of r
	uint8_t g_up2 = g_up4 >> 2;				// upper 2 bits of g
	uint8_t b_up2 = b_up4 >> 2;				// upper 2 bits of b

	uint8_t r_lo2 = (r & LOW1BITS) << 1;	// lower 2 bits of r (appended 0 so R1,0)
	uint8_t g_lo2 = (g & LOW2BITS);			// lower 2 bits of g
	uint8_t b_lo2 = (b & LOW1BITS) << 1;	// lower 2 bits of b (appended 0 so B1,0)

	// get the index to the level 4 array
	uint16_t lvl4_idx = (r_up4 << 8) | (g_up4 << 4) | b_up4;

	// get the index to the level 2 array
	uint8_t lvl2_idx = (r_up2 << 4) | (g_up2 << 2) | b_up2;

	// bump up the histogram for level 4
	lvl_4[lvl4_idx].occur += 1;

	// update the mean values
	lvl_4[lvl4_idx].r_mean += r_lo2;
	lvl_4[lvl4_idx].g_mean += g_lo2;
	lvl_4[lvl4_idx].b_mean += b_lo2;

	// bump up the histogram for level 2
	lvl_2[lvl2_idx].occur += 1;

	// update the means for level 2
	lvl_2[lvl2_idx].r_mean += (r & 0x07) << 1;
	lvl_2[lvl2_idx].g_mean += g & 0x0F;
	lvl_2[lvl2_idx].b_mean += (b & 0x07) << 1;
}

/*
 * compare_octree
 *   DESCRIPTION: compare two octree objects
 *   INPUTS: a - number 1
 			 b - number 2
 *   OUTPUTS: the difference in occurance
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */  
int compare_octree(const void * a, const void * b){
   return (((octree_t*)b)->occur - ((octree_t*)a)->occur);
}

/*
 * create_palette192
 *   DESCRIPTION: make palette of 192 colors
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: arranges 2D array into 1D array to send to modex
 */  
void create_palette192(){
	// counter
	int i;

	// create array for sorted octree lvl 4
	octree_t sorted_lvl4[4096];

	for(i = 0; i < 4096; i++){
		sorted_lvl4[i] = lvl_4[i];
	}

	// sort the level 4 array 
	qsort((void*)sorted_lvl4, (int)sizeof(octree_t), 0, 4095, (void*)compare_octree);

	// loop through the first 128 top colors
	for(i = 0; i < 128; i++){
		// calculate the index for the level 2 tree
		uint8_t r_up2 = sorted_lvl4[i].r >> 2;				// upper 2 bits of r
		uint8_t g_up2 = sorted_lvl4[i].g >> 2;				// upper 2 bits of g
		uint8_t b_up2 = sorted_lvl4[i].b >> 2;				// upper 2 bits of b
		uint8_t lvl2_idx = (r_up2 << 4) | (g_up2 << 2) | b_up2;

		// subtract the contribution from the level 2 tree
		lvl_2[lvl2_idx].occur -= sorted_lvl4[i].occur;
		lvl_2[lvl2_idx].r_mean -= ((sorted_lvl4[i].r & LOW2BITS) << 2) * sorted_lvl4[i].occur + sorted_lvl4[i].r_mean;
		lvl_2[lvl2_idx].g_mean -= ((sorted_lvl4[i].g & LOW2BITS) << 2) * sorted_lvl4[i].occur + sorted_lvl4[i].g_mean;
		lvl_2[lvl2_idx].b_mean -= ((sorted_lvl4[i].b & LOW2BITS) << 2) * sorted_lvl4[i].occur + sorted_lvl4[i].b_mean;

		// calculate the mean for the sorted level 4 array unless divide by 0 error
		sorted_lvl4[i].r_mean = sorted_lvl4[i].occur != 0 ? sorted_lvl4[i].r_mean / sorted_lvl4[i].occur : 0;
		sorted_lvl4[i].g_mean = sorted_lvl4[i].occur != 0 ? sorted_lvl4[i].g_mean / sorted_lvl4[i].occur : 0;
		sorted_lvl4[i].b_mean = sorted_lvl4[i].occur != 0 ? sorted_lvl4[i].b_mean / sorted_lvl4[i].occur : 0;

		// get the 6 bits for each color and add them to the palette
		palette[i * 3 + 0] = ((sorted_lvl4[i].r & LOW4BITS) << 2) | (sorted_lvl4[i].r_mean & LOW2BITS);
		palette[i * 3 + 1] = ((sorted_lvl4[i].g & LOW4BITS) << 2) | (sorted_lvl4[i].g_mean & LOW2BITS);
		palette[i * 3 + 2] = ((sorted_lvl4[i].b & LOW4BITS) << 2) | (sorted_lvl4[i].b_mean & LOW2BITS);

		// get the 12 bit color index into the original level 4 array
		uint16_t color = (sorted_lvl4[i].r << 8) | (sorted_lvl4[i].g << 4) | sorted_lvl4[i].b;

		// set the palette index in the original level 4 tree
		lvl_4[color].pal_idx = i + 64;						
	}

	// loop through the 64 colors for the level 2 array
	for(i = 0; i < 64; i++){
		// set the mean to the right value
		lvl_2[i].r_mean = lvl_2[i].occur != 0 ? lvl_2[i].r_mean / lvl_2[i].occur : 0;
		lvl_2[i].g_mean = lvl_2[i].occur != 0 ? lvl_2[i].g_mean / lvl_2[i].occur : 0;
		lvl_2[i].b_mean = lvl_2[i].occur != 0 ? lvl_2[i].b_mean / lvl_2[i].occur : 0;

		// get the 6 bits for each color and add them to the palette
		palette[(i + 128) * 3 + 0] = (lvl_2[i].r << 4) | (lvl_2[i].r_mean & LOW4BITS);
		palette[(i + 128) * 3 + 1] = (lvl_2[i].g << 4) | (lvl_2[i].g_mean & LOW4BITS);
		palette[(i + 128) * 3 + 2] = (lvl_2[i].b << 4) | (lvl_2[i].b_mean & LOW4BITS);

		// set the palette value to the level 2 octree
		lvl_2[i].pal_idx = i + 192;						
	}
}
