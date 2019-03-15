#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "png_assist.h"

void getARGB(uint8_t * argb, uint32_t extracted){
	argb[0] = extracted >> 24;
	argb[1] = (extracted >> 16) % (1 << 8);
	argb[2] = (extracted >> 8) % (1 << 8);
	argb[3] = extracted % (1 << 8);
	return;
}


//Account for invert here
int load_png(char * source, uint32_t ** buffer_mono, bool invert){
	uint16_t buff_width = 48;
	uint16_t buff_height = 32;
	*buffer_mono = malloc(sizeof(uint32_t) * buff_width * buff_height);
	if(!(*buffer_mono)){printf("Ran out of memory. Terminating now\n"); return 1;}
	uint32_t * current_texel = *buffer_mono;
	if(invert){
		current_texel += (48 * 31);
	}
	png_details_t p_det;

	if(read_png_file(source, &p_det)){printf("File %s couldn't be read. Terminating now\n", source); return 2;}
	if(p_det.width != buff_width){
		printf("%s has incorrect width\nWidth should be 72\nThis image's width is %d\n", source, p_det.width);
		return 3;
	}
	if(p_det.height != buff_height){
		printf("%s has incorrect height\nHeight should be 56\nThis image's height is %d\n", source, p_det.height);
		return 3;
	}

	for(int y = 0; y < p_det.height; y++){
		for(int x = 0; x < p_det.width; x++){
			png_bytep px = &(p_det.row_pointers[y][x * 4]);
			current_texel[0] = (px[0] << 24) + (px[1] << 16) + (px[2] << 8) + px[3];
			current_texel++;
			if(invert && x == p_det.width - 1){
				current_texel -= 2 * p_det.width;
			}
		}
	}

	free_png_texture_buffer(&p_det);

	return 0;
}

int make_binary(char * dest, uint32_t * buffer_mono, uint8_t ** buffer_binary, uint16_t frames){
	return 0;
}

int make_png(char * dest, uint32_t * buffer_mono){
	png_details_t p_det;
	p_det.height = 32;
	p_det.width = 48;

	p_det.color_type = PNG_COLOR_MASK_COLOR + PNG_COLOR_MASK_ALPHA;	//= 2 + 4 = 6. They describe the color_type field in png_info
	p_det.bit_depth = 8;	//rgba8888, can be 1, 2, 4, 8, or 16 bits/channel (from IHDR)

	//Allocate space
	p_det.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * p_det.height);
	if(!p_det.row_pointers){printf("Ran out of memory. Terminating now\n"); return 1;}
	for(int y = 0; y < p_det.height; y++){
		p_det.row_pointers[y] = (png_byte*)malloc(sizeof(png_byte) * p_det.width * 4);
		if(!p_det.row_pointers[y]){
			for(int i = 0; i < y; i++){	//Free the rest of the array
				free(p_det.row_pointers[i]);
			}
			free(p_det.row_pointers);
			printf("Ran out of memory. Terminating now\n");
			return 1;
		}
	}

	for(int y = 0; y < p_det.height; y++){
		for(int x = 0; x < p_det.width; x++){
			png_bytep px = &(p_det.row_pointers[y][x * 4]);

			px[0] = bit_extracted(buffer_mono[(y * p_det.width) + x], 8, 24);
			px[1] = bit_extracted(buffer_mono[(y * p_det.width) + x], 8, 16);
			px[2] = bit_extracted(buffer_mono[(y * p_det.width) + x], 8, 8);
			px[3] = bit_extracted(buffer_mono[(y * p_det.width) + x], 8, 0);
		}
	}
	write_png_file(dest, &p_det);	//Free-s the struct once drawn
	return 0;
}

// int makeVmuLcdIcon(char * source, char * dest, char * png, bool invert){
// 	int x, y, n;
// 	stbi_set_flip_vertically_on_load(invert);	//Since the VMU is upside down in the controller,
// 											//this flips the image upside down so it appears correctly

// 	unsigned char *data = stbi_load(source, &x, &y, &n, 0);	//x is width, y is height, n is number of channels
// 																	//Last param is zero for default ARGB8888

// 	if(data == 0){
// 		fprintf(stderr, "\nCouldn't load the requested file\n");
// 		return 1;
// 	}
// 	if(n != 3 && n != 4){	//Not argb or rgb
// 		fprintf(stderr, "\nImage pixels are not A/RGB. Texture may not have loaded correctly. (%d channels)\n", n);
// 		return 1;
// 	}

// 	FILE *write_ptr;
// 	write_ptr = fopen(dest,"wb");  // w for write, b for binary
// 	if(write_ptr == NULL){
// 		fprintf(stderr, "\nFile %s cannot be opened\n", dest);
// 		return 1;
// 	}
// 	unsigned char *traversal = data;
// 	uint32_t extracted;
// 	uint8_t buffer[6];	//We need to output in Little-endian so we have to store 6 bytes per row and then write to the file
// 	uint8_t buffer_count = 0;

// 	uint8_t argb[4];
// 	uint8_t * colour = argb;	//For some reason I can't pass a reference to argb into getARGB() :roll_eyes:

// 	for(int i = 0; i < x * y / 8; i++){	//We have a for-loop that goes 8 times within this loop
// 		buffer[5- buffer_count] = 0;

// 		//Need to pack 8 pixels into one byte (One element of buffer[])
// 		for(int j = 0; j < 8; j++){
// 			memcpy(&extracted, traversal, n);	//Extracts all 4 bytes (RGB888 is converted to ARGB8888 with stbi_load(). Alpha channel is zero)

// 			getARGB(colour, extracted);

// 			//If the colours are "strong enough and there's enough opaqueness (Or we have RGB)" then add a black pixel
// 			if(argb[1] + argb[2] + argb[3] < (255/2) * 3 && (argb[0] > (255/2) || n == 3)){
// 				buffer[5 - buffer_count] |= (1 << j);	//VMU icons use 0 for no pixel and 1 for pixel
// 			}
// 			traversal += sizeof(uint8_t) * n;
// 		}

// 		buffer_count++;

// 		//When we've read a row, write to file
// 		if(buffer_count == 6){
// 			fwrite(buffer, sizeof(uint8_t), sizeof(buffer), write_ptr);
// 			buffer_count = 0;
// 		}
// 	}

// 	fclose(write_ptr);
// 	stbi_image_free(data);

// 	return 0;
// }

void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./VmuLcdIconCreator --input [png_filename] --output-binary [filename] *--output-png [filename] *--invert\n\n");
	printf("Note you can only have 1 png file\nThe PNG must be 48 pixels wide and 32 pixels high\n");
	printf("Preview is optional and just gives you a preview of the binary\n");

	exit(1);
}

//Just to make it more obvious to people viewing the code what is happening
bool string_equals(char * a, char * b){
	return !strcmp(a, b);
}

int main(int argC, char *argV[]){
	bool flag_input_image = false;
	bool flag_output_binary = false;
	bool flag_output_png = false;
	bool flag_invert = false;
	uint8_t input_image_index = 0;
	uint8_t output_binary_index = 0;
	uint8_t output_png_index = 0;
	for(int i = 1; i < argC; i++){
		if(string_equals(argV[i], "--input")){
			if(++i >= argC){
				invalid_input();
			}
			flag_input_image = true;
			input_image_index = i;
		}
		else if(string_equals(argV[i], "--output-binary")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output_binary = true;
			output_binary_index = i;
		}
		else if(string_equals(argV[i], "--output-png")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output_png = true;
			output_png_index = i;
		}
		else if(string_equals(argV[i], "--invert")){
			flag_invert = true;
		}
	}
	if(!flag_input_image || !flag_output_binary){
		invalid_input();
	}

	uint32_t * buffer_mono = NULL;
	uint8_t * buffer_binary = NULL;
	uint16_t frames = 1;

	load_png(argV[input_image_index], &buffer_mono, flag_invert);
	make_png(argV[output_png_index], buffer_mono);
	make_binary(argV[output_binary_index], buffer_mono, &buffer_binary, frames);

	free(buffer_mono);

	return 0;
}
