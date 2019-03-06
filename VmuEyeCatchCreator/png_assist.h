#ifndef PNG_ASSIST_H
#define PNG_ASSIST_H

#include <png.h>
#include <stdlib.h>
#include <stdint.h>
// #include <stdio.h>

typedef struct png_details{
	int width, height;
	png_bytep *row_pointers;

	//These two are only used in read
	png_byte color_type;
	png_byte bit_depth;
} png_details_t;

//The content of these functions comes from this post on github by niw
	//https://gist.github.com/niw/5963798
int read_png_file(char *filename, png_details_t * p_det);
void free_png_texture_buffer(png_details_t * p_det);
void write_png_file(char *filename, png_details_t * p_det);

int bit_extracted(uint32_t number, int k, int p);
uint8_t rgba8888_to_png_details(uint32_t * pixel_data, int height, int width, png_details_t * p_det);

#endif
