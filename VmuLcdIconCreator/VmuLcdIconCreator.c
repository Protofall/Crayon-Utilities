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

int makeVmuLcdIcon(char * source, char * dest, char * png, bool invert){
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

	makeVmuLcdIcon(argV[input_image_index], argV[output_binary_index], argV[output_png_index], flag_invert);

	return 1;

	// //Check input for options
	// bool flag_invert;
	// bool flag_help;
	// for(int i = 1; i < argC; i++){
	// 	!strcmp(argV[i], "--invert") ? flag_invert = true : flag_invert = false;
	// 	!strcmp(argV[i], "--help") ? flag_help = true : flag_help = false;
	// }

	// if(flag_help || (argC != 3 && argC != 4)){
	// 	printf("\nUsage:\n./VmuLcdIconCreator [Source_file_path] [Dest_file_path] [options]\n\nList of potential options:\n");
	// 	printf("--invert	Flips y/height axis. Note VMU is upside down in a DC controller\n");
	// 	printf("--help		Display this message\n");
	// 	return 1;
	// }

	// printf("\nPlease note this is only confirmed to work with RGB888 and ARGB8888 textures\n");
	// printf("Any other format and the resulting image might not work right\n");

	// makeVmuLcdIcon(argV[1], argV[2], flag_invert);
	
	// return 0;
}
