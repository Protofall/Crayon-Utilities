#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "png_assist.h"

#define BIN_WIDTH 48
#define BIN_HEIGHT 32

//Sets the value of a pixel
void set_pixel(uint32_t * texel, uint8_t value){
	(*texel) = (value << 24) + (value << 16) + (value << 8) + 0xFF;
	return;
}

int read_binary(char * dest, uint32_t * texture, uint8_t frame_count){
	int x = BIN_WIDTH;
	int y = BIN_HEIGHT;
	uint16_t size_frame = BIN_HEIGHT * BIN_WIDTH / 8;	//In bytes

	FILE * fp = fopen(dest,"rb");
	if(!fp){
		fprintf(stderr, "\nFile %s cannot be opened\n", dest);
		return 1;
	}

	// printf("%d %d %d %d\n", size_frame, frame_count, BIN_HEIGHT, BIN_WIDTH);
	uint8_t buffer;
	int index;
	for(int i = 0; i < size_frame * frame_count; i++){	//We have a for-loop that goes 8 times within this loop
		fread(&buffer, sizeof(uint8_t), 1, fp);

		//Unpack the 8 pixels from the one byte
		for(int j = 0; j < 8; j++){
			index = (8 * ((5 - i%6) + i - i%6)) + j;		//Need to reverse every 6 bytes
			if(buffer & (1 << j)){
				set_pixel(&texture[index], 0);
			}
			else{
				set_pixel(&texture[index], 255);

			}
		}
	}

	fclose(fp);

	return 0;
}

void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./LcdBinToPng --input [binary_filename] --output [png_filename] *--mode [0 || 1]\n\n");
	printf("Input and output are self explanitory, mode optional, but is currently unused\n");

	exit(1);
}

//Just to make it more obvious to people viewing the code what is happening
bool string_equals(char * a, char * b){
	return !strcmp(a, b);
}

int main(int argC, char *argV[]){
	bool flag_input = false;
	bool flag_output = false;
	uint8_t input_index;
	uint8_t output_index;
	uint8_t mode = 0;
	for(int i = 1; i < argC; i++){
		if(string_equals(argV[i], "--input")){
			if(++i >= argC){
				invalid_input();
			}
			flag_input = true;
			input_index = i;
		}
		if(string_equals(argV[i], "--output")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output = true;
			output_index = i;
		}
		if(string_equals(argV[i], "--mode")){
			printf("--mode is currently unused\n");
			if(++i >= argC){
				invalid_input();
			}
			mode = atoi(argV[i]);
			if(mode == 0 || mode == 1){
				invalid_input();
			}
		}
	}
	if(!flag_input || !flag_output){
		invalid_input();
	}

	size_t size_binary = 0;	//Each byte = 8 pixels. 1 frame is 48 by 32 so thats 192 bytes (1536 pixels)
	FILE * fp = fopen(argV[input_index],"rb");  //r for read, b for binary
	fseek(fp, 0, SEEK_END);
	size_binary = ftell(fp);
	fclose(fp);
	uint16_t size_frame = BIN_HEIGHT * BIN_WIDTH / 8;
	if(size_binary % size_frame){
		printf("Binary isn't in the correct format, size must be a multiple of 192 bytes\n");
		return 1;
	}

	uint8_t frames = size_binary / size_frame;
	uint32_t * texture = malloc(sizeof(uint32_t) * frames * size_frame * 8);	//Times 8 to convert from byte to pixels

	read_binary(argV[input_index], texture, frames);

	png_details_t p_det;
	if(rgba8888_to_png_details(texture, BIN_HEIGHT * frames, BIN_WIDTH, 1, &p_det)
		|| write_png_file(argV[output_index], &p_det)){
		printf("Error occured when creating PNG\n");
		free(texture);
		return 1;
	}

	free(texture);
	return 0;
}
