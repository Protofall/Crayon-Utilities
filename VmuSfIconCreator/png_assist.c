#include "png_assist.h"

int read_png_file(char *filename, png_details_t * p_det){
	FILE *fp = fopen(filename, "rb");
	if(!fp){return 1;}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png){abort();}

	png_infop info = png_create_info_struct(png);
	if(!info){abort();}

	if(setjmp(png_jmpbuf(png))){abort();}

	png_init_io(png, fp);

	png_read_info(png, info);

	p_det->width      = png_get_image_width(png, info);
	p_det->height     = png_get_image_height(png, info);
	p_det->color_type = png_get_color_type(png, info);
	p_det->bit_depth  = png_get_bit_depth(png, info);

	// printf("Here are the values from this horridly documented library: %d, %d\n", p_det->color_type, p_det->bit_depth);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if(p_det->bit_depth == 16){
		png_set_strip_16(png);
	}

	if(p_det->color_type == PNG_COLOR_TYPE_PALETTE){
		png_set_palette_to_rgb(png);
	}

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(p_det->color_type == PNG_COLOR_TYPE_GRAY && p_det->bit_depth < 8){
		png_set_expand_gray_1_2_4_to_8(png);
	}

	if(png_get_valid(png, info, PNG_INFO_tRNS)){
		png_set_tRNS_to_alpha(png);
	}

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(p_det->color_type == PNG_COLOR_TYPE_RGB ||
		 p_det->color_type == PNG_COLOR_TYPE_GRAY ||
		 p_det->color_type == PNG_COLOR_TYPE_PALETTE){
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if(p_det->color_type == PNG_COLOR_TYPE_GRAY ||
		 p_det->color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
		png_set_gray_to_rgb(png);
	}

	png_read_update_info(png, info);

	p_det->row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * p_det->height);
	for(int y = 0; y < p_det->height; y++){
		// png_uint_32 value = png_get_rowbytes(png,info);	//Seems to be equal to 4 times width
		// printf("%d: %d\n", y, value);
		// p_det->row_pointers[y] = (png_byte*)malloc(value);
		p_det->row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	png_read_image(png, p_det->row_pointers);

	png_destroy_read_struct(&png, &info, NULL);
    png = NULL;
    info = NULL;

	fclose(fp);

	return 0;
}

void free_png_texture_buffer(png_details_t * p_det){
	for(int y = 0; y < p_det->height; y++){
		free(p_det->row_pointers[y]);
	}

	free(p_det->row_pointers);
	return;
}

void write_png_file(char *filename, png_details_t * p_det){
	int y;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	//Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if(!fp){abort();}

	//Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){abort();}

	//Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){abort();}

	//Setup Exception handling
	if(setjmp(png_jmpbuf(png_ptr))){abort();}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, p_det->width, p_det->height,
		8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
	);
	// png_set_IHDR(png_ptr, info_ptr, p_det->width, p_det->height,
	// 	8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
	// 	PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE
	// );
	png_write_info(png_ptr, info_ptr);

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	//png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	png_write_image(png_ptr, p_det->row_pointers);

	png_write_end(png_ptr, NULL);

	free_png_texture_buffer(p_det);

	if(png_ptr && info_ptr){
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}

	fclose(fp);
}

// k lots of 1's bitwise and with your number moved right by p bits
int bit_extracted(uint32_t number, int k, int p){
	return (((1 << k) - 1) & (number >> p)); 
}

uint8_t rgba8888_to_png_details(uint32_t * pixel_data, int height, int width, png_details_t * p_det){
	p_det->height = height;
	p_det->width = width;

	p_det->color_type = PNG_COLOR_MASK_COLOR + PNG_COLOR_MASK_ALPHA;	//= 2 + 4 = 6. They describe the color_type field in png_info
	p_det->bit_depth = 8;	//rgba8888, can be 1, 2, 4, 8, or 16 bits/channel (from IHDR)

	//Allocate space
	p_det->row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * p_det->height);
	if(!p_det->row_pointers){printf("Ran out of memory. Terminating now\n"); return 1;}
	for(int y = 0; y < p_det->height; y++){
		p_det->row_pointers[y] = (png_byte*)malloc(sizeof(png_byte) * p_det->width * 4);
		if(!p_det->row_pointers[y]){
			for(int i = 0; i < y; i++){	//Free the rest of the array
				free(p_det->row_pointers[i]);
			}
			free(p_det->row_pointers);
			printf("Ran out of memory. Terminating now\n");
			return 1;
		}
	}

	for(int y = 0; y < p_det->height; y++){
		for(int x = 0; x < p_det->width; x++){

			png_bytep px = &(p_det->row_pointers[y][x * 4]);
			// printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
			px[0] = bit_extracted(pixel_data[(y * width) + x], 8, 24);	//R
			px[1] = bit_extracted(pixel_data[(y * width) + x], 8, 16);	//G
			px[2] = bit_extracted(pixel_data[(y * width) + x], 8, 8);	//B
			px[3] = bit_extracted(pixel_data[(y * width) + x], 8, 0);	//A
		}
	}
	return 0;
}

//from pngconf.h

//typedef png_byte * png_bytep;
//typedef png_byte * * png_bytepp;

//Somewhat useful
//https://linux.die.net/man/3/libpng
