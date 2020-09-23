#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "png_assist.h"

#define BIN_WIDTH 48
#define BIN_HEIGHT 32

// Account for invert here
int load_png(char * source, uint32_t ** buffer_mono, bool invert){
	uint16_t buff_width = BIN_WIDTH;
	uint16_t buff_height = BIN_HEIGHT;
	*buffer_mono = malloc(sizeof(uint32_t) * buff_width * buff_height);
	// *buffer_mono = calloc(buff_width * buff_height, sizeof(uint32_t));
	if(!(*buffer_mono)){printf("Ran out of memory. Terminating now\n"); return 1;}
	uint32_t * current_texel = *buffer_mono;
	if(invert){
		current_texel += (BIN_WIDTH * (BIN_HEIGHT - 1));
	}
	png_details_t p_det;

	if(read_png_file(source, &p_det)){printf("File %s couldn't be read. Terminating now\n", source); return 2;}
	if(p_det.width != buff_width){
		printf("%s has incorrect width. Width should be %d, but this image's width is %d\n", source, buff_width, p_det.width);
		free(buffer_mono);
		return 2;
	}
	if(p_det.height != buff_height){
		printf("%s has incorrect height. Height should be %d, but this image's height is %d\n", source, buff_height, p_det.height);
		free(buffer_mono);
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
		#else
			#error LUMA_CALC undefined!
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

double thresholdFunc(uint16_t top, uint16_t bottom){
	return 255.0 * (top / (double)bottom);
}

void make_binary(uint8_t **bin, uint32_t *buffer_mono, uint8_t frame_count){
	uint32_t x = BIN_WIDTH;
	uint32_t y = BIN_HEIGHT;

	// The size of a frame in bytes
	uint32_t size_frame = x * y / 8;

	//Divide by 8 because each byte contains 8 pixels
	*bin = malloc(sizeof(uint8_t) * size_frame * frame_count);

	uint8_t six_bytes[6] = {0};	//Contains 6 * 8 = 48 pixels

	// For 1 frame, the threshold should be 127.5, but ATM its 255
		// 2 frames would be 85, 170
	// Then get the pixel value, see which value its closest to
		// f_c = 1, points = 2, threshold = 127.5
		// f_c = 2, points = 3, threshold = 170, 85

	uint8_t value;
	uint32_t bin_pixel_id = 0;
	uint16_t points = frame_count + 1;
	double threshold;
	double lower, upper;
	for(int f = 0; f < frame_count; f++){	// Num frames
		threshold = thresholdFunc(points - f - 1, points);

		int b = 0;
		int c = 0;
		for(int a = 0; a < x * y; a++){	// Number of pixels in a frame
			value = (buffer_mono[a] >> 24);	//This is the red channel, but its all grey so save as G and B

			lower = thresholdFunc(points - f - 1, points);	//current one
			upper = thresholdFunc(points - f - 1 + 1, points);
			if(value <= lower){
				six_bytes[5 - b] |= (1 << c);
			}

			c++;
			if(c == 8){
				c = 0;
				b++;
				if(b == 6){
					b = 0;
					// Time to append the 6 bytes to the bin
					for(int d = 0; d < 6; d++){
						(*bin)[bin_pixel_id] = six_bytes[d];
						six_bytes[d] = 0;
						bin_pixel_id++;
					}
				}
			}
		}
	}

	return;
}

void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./VmuLcdIconCreator --input [png_filename] --output-binary [filename] *--invert *--png-preview [filename] *--png-mode [mode] *--scale [scale factor] *--frames [frame count]\n\n");
	printf("Invert is optional, this will flip the vertical axis of the image. Enable if using it in a Dreamcast controller\n");
	printf("Frames is the number of LCD icon frames the binary will contain. By default its 1 and it can go up to 255 (More info in readme)\n");
	printf("Png output is optional and just gives you a preview of the monochrome image that should be produced\n");
	printf("Note you can only have 1 png file, the PNG must be 48 pixels wide and 32 pixels high\n");
	printf("Png mode is used for greyscale. It will either give you a png output of the greyscale preview (0) or just the entire binary with each individual frame (1). Default is 0.\n");
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
	uint8_t png_mode = 0;
	for(int i = 1; i < argC; i++){
		if(string_equals(argV[i], "--input") || string_equals(argV[i], "-i")){
			if(++i >= argC){
				invalid_input();
			}
			flag_input_image = true;
			input_image_index = i;
		}
		else if(string_equals(argV[i], "--output-binary") || string_equals(argV[i], "-o")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output_binary = true;
			output_binary_index = i;
		}
		else if(string_equals(argV[i], "--frames") || string_equals(argV[i], "-f")){
			if(++i >= argC){
				invalid_input();
			}
			flag_frames = true;
			frame_count = atoi(argV[i]);
			if(frame_count < 1 || frame_count > 255){
				invalid_input();
			}
		}
		else if(string_equals(argV[i], "--scale") || string_equals(argV[i], "-s")){
			if(++i >= argC){
				invalid_input();
			}
			flag_scale = true;
			scale = atoi(argV[i]);
		}
		else if(string_equals(argV[i], "--png-preview") || string_equals(argV[i], "-p")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output_png = true;
			output_png_index = i;
		}
		else if(string_equals(argV[i], "--png-mode") || string_equals(argV[i], "--pm")){
			if(++i >= argC){
				invalid_input();
			}
			png_mode = atoi(argV[i]);
		}
		else if(string_equals(argV[i], "--invert") || string_equals(argV[i], "--inv")){
			flag_invert = true;
		}
	}
	if(!flag_input_image || !flag_output_binary || png_mode > 1){
		invalid_input();
	}

	uint32_t *buffer_mono = NULL;

	load_png(argV[input_image_index], &buffer_mono, flag_invert);
	convert_to_monochrome(buffer_mono);
	reduce_colour_range(buffer_mono, frame_count);	// Assumes frame_count = 1. Will still work, but could be done better
	
	// Create the binary
	uint8_t *binary = NULL;
	make_binary(&binary, buffer_mono, frame_count);

	// Write the binary
	FILE *write_ptr = fopen(argV[output_binary_index],"wb");
	if(!write_ptr){
		fprintf(stderr, "\nFile %s cannot be opened\n", argV[output_binary_index]);
		free(binary);
		return 1;
	}
	fwrite(binary, sizeof(uint8_t), BIN_HEIGHT * BIN_WIDTH * frame_count / 8, write_ptr);
	fclose(write_ptr);

	// Free it
	free(binary);

	// Write the PNG
	if(flag_output_png){
		png_details_t p_det;
		if(rgba8888_to_png_details(buffer_mono, BIN_HEIGHT, BIN_WIDTH, scale, &p_det)
			|| write_png_file(argV[output_png_index], &p_det)){
			printf("Failed to create PNG\n");
			free(buffer_mono);
			return 1;
		}
	}

	free(buffer_mono);

	return 0;
}
