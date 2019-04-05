#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "png_assist.h"

#define BIN_WIDTH 48
#define BIN_HEIGHT 32

//Account for invert here
int load_png(char * source, uint32_t ** buffer_mono, bool invert){
	uint16_t buff_width = BIN_WIDTH;
	uint16_t buff_height = BIN_HEIGHT;
	*buffer_mono = malloc(sizeof(uint32_t) * buff_width * buff_height);
	if(!(*buffer_mono)){printf("Ran out of memory. Terminating now\n"); return 1;}
	uint32_t * current_texel = *buffer_mono;
	if(invert){
		current_texel += (BIN_WIDTH * (BIN_HEIGHT - 1));
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

//Input is in RGBA format
//0 is A, 1 is R, 2 is G, 3 is B which is ARGB format
//Also because I want to pass in an array of a fixed size, I must use it like so
void getARGB(uint8_t (*argb)[4], uint32_t extracted){
	(*argb)[0] = bit_extracted(extracted, 8, 0);
	(*argb)[1] = bit_extracted(extracted, 8, 24);
	(*argb)[2] = bit_extracted(extracted, 8, 16);
	(*argb)[3] = bit_extracted(extracted, 8, 8);
	return;
}

//Takes an rgba8888 pixel and a monochrome value and updates the pixel
void update_monochrome(uint32_t * texel, uint8_t monochrome){
	(*texel) = (monochrome << 24) + (monochrome << 16) + (monochrome << 8) + ((*texel) & 0xFF);
	return;
}

#define LUMA_CALC 2	//Change this if you want a different luma calc, this one is my favourite

//Converts all pixels to a monochrome colour
//Monochrome calculations from:
//		https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
int convert_to_monochrome(uint32_t * buffer_mono){
	uint8_t width = BIN_WIDTH;
	uint8_t height = BIN_HEIGHT;
	uint8_t argb[4];
	uint32_t extracted;
	float monochrome;	//255 is white, 0 is black
	for(int i = 0; i < height * width; i++){
		getARGB(&argb, buffer_mono[i]);

		//If too trasparent, treat it as pure white/transparent
		if(argb[0] < (float)255/(float)2.0){
			buffer_mono[i] = 0xFFFFFFFF;	//White
			continue;
		}

		#if LUMA_CALC == 0
			monochrome = ((0.2126*argb[1]) + (0.7152*argb[2]) + (0.0722*argb[3]));
		#elif LUMA_CALC == 1
			monochrome = ((0.299*argb[1]) + (0.587*argb[2]) + (0.114*argb[3]));
		#elif LUMA_CALC == 2
			monochrome = sqrt((0.299*pow(argb[1], 2)) + (0.587*pow(argb[2], 2)) + (0.114*pow(argb[3], 2)));	//Gives more accurate monochrome
		#endif

		update_monochrome(&buffer_mono[i], (int)monochrome);
	}

	return 0;
}

//Checks to see if "val" is closer to bound 1 or 2
float closest(float bound1, float bound2, uint32_t val){
	if(bound1 < 0){
		bound1 = 0;
	}
	if(bound2 < 0){
		bound2 = 0;
	}

	float a = fabs(bound1-val);
	float b = fabs(bound2-val);
	if(a < b){
		return bound1;
	}

	return bound2;
}

//Currently this just re-reduces to 1BPP
void reduce_colour_range(uint32_t * buffer_mono, uint8_t frame_count){
	//frame_count 3. 0, 85, 170 and 255
	//Mine is: 0, 128, 192, 255. This would default to 0, 170 and 255 (170-128 < 128-85)
	//frame_count 4. 0, 64, 128, 192, 255. This would give all colours for Mine easily (frame 3 and 4 would be the same)

	frame_count = 1;	//Until the binary creator function has support for multiple frames, we must limit it

	uint16_t colours = frame_count + 1;	//+1 because we have one extra colour, no colour
	double step = 255.0/(double)frame_count;	//Always ranging between 255 and 1
	float temp1, temp2;
	uint8_t temp3, temp4;

	float a, b;
	uint8_t c, d;
	for(int i = 0; i < BIN_WIDTH * BIN_HEIGHT; i++){
		c = buffer_mono[i] >> 24;	//Get the monochrome value
		float a = c - fmod(c,step);
		float b = a + step;
		a = ceil(a);
		b = ceil(b);
		uint8_t d = closest(a, b, c);

		update_monochrome(&buffer_mono[i], d);
	}
	return;
}

int make_binary(char * dest, uint32_t * buffer_mono, uint8_t frame_count){
	int x = BIN_WIDTH;
	int y = BIN_HEIGHT;
	FILE * write_ptr = fopen(dest,"wb");  // w for write, b for binary
	if(!write_ptr){
		fprintf(stderr, "\nFile %s cannot be opened\n", dest);
		return 1;
	}

	// uint32_t extracted;
	uint8_t buffer[6];	//We need to output in Little-endian so we have to store 6 bytes per row and then write to the file
	uint8_t buffer_count = 0;

	for(int i = 0; i < x * y / 8; i++){	//We have a for-loop that goes 8 times within this loop
		buffer[5- buffer_count] = 0;

		//Need to pack 8 pixels into one byte (One element of buffer[])
		for(int j = 0; j < 8; j++){
			// memcpy(&extracted, buffer_mono, sizeof(uint32_t));

			//If pixel is black, set pixel to true
			if(buffer_mono[0] == 0xFF){
				buffer[5 - buffer_count] |= (1 << j);
			}
			buffer_mono++;
		}

		buffer_count++;

		//When we've read a row, write to file
		if(buffer_count == 6){
			fwrite(buffer, sizeof(uint8_t), sizeof(buffer), write_ptr);
			buffer_count = 0;
		}
	}

	fclose(write_ptr);

	return 0;
}

void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./VmuLcdIconCreator --input [png_filename] --output-binary [filename] *--invert *--output-png [filename] *--scale *--frames [frame count]\n\n");
	printf("Invert is optional, this will flip the vertical axis of the image. Enable if using it in a Dreamcast controller\n");
	printf("Frames is the number of LCD icon frames the binary will contain. By default its 1 and it can go up to 255 (More info in readme)\n");
	printf("Png output is optional and just gives you a preview of the monochrome image that should be produced\n");
	printf("Note you can only have 1 png file, the PNG must be 48 pixels wide and 32 pixels high\n");
	printf("Scale is optional, this will increase the size of the output PNG by the factor squared provided. Default is 1\n");

	exit(1);
}

//Just to make it more obvious to people viewing the code what is happening
bool string_equals(char * a, char * b){
	return !strcmp(a, b);
}

int main(int argC, char *argV[]){
	bool flag_input_image = false;
	bool flag_output_binary = false;
	bool flag_frames = false;
	bool flag_scale = false;
	bool flag_output_png = false;
	bool flag_invert = false;
	uint8_t input_image_index = 0;
	uint8_t output_binary_index = 0;
	uint8_t frame_count = 1;
	uint8_t scale = 1;
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
		if(string_equals(argV[i], "--frames")){
			if(++i >= argC){
				invalid_input();
			}
			flag_frames = true;
			frame_count = atoi(argV[i]);
			if(frame_count < 1 || frame_count > 255){
				invalid_input();
			}
		}
		else if(string_equals(argV[i], "--scale")){
			if(++i >= argC){
				invalid_input();
			}
			flag_scale = true;
			scale = atoi(argV[i]);
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

	load_png(argV[input_image_index], &buffer_mono, flag_invert);
	convert_to_monochrome(buffer_mono);
	reduce_colour_range(buffer_mono, frame_count);	//Currently assumes frame_count = 1;
	make_binary(argV[output_binary_index], buffer_mono, frame_count);
	if(flag_output_png){
		png_details_t p_det;
		if(rgba8888_to_png_details(buffer_mono, BIN_HEIGHT, BIN_WIDTH, scale, &p_det)
			|| write_png_file(argV[output_png_index], &p_det)){
			printf("Failed to create PNG\n");
			return 1;
		}
	}

	free(buffer_mono);

	return 0;
}
