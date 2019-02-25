#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "png_assist.h"


void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	// printf("./DtexToRGBA8888 [dtex_filename] *[--binary] [rgba8888_binary_filename] *[--png] [png_filename]\n");
	// printf("png and binary is an optional tag that toggle png and/or binary outputs\n");
	// printf("However you must choose at least one\n");

	exit(1);
}

//Just to make it more obvious to people viewing the code what is happening
bool string_equals(char * a, char * b){
	return !strcmp(a, b);
}

// uint8_t png_to_dc_savefile_binary(uint8_t input_image_index_start, uint8_t num_frames,
// 		uint8_t output_image_index, uint8_t output_palette_index, char * argV[]){
// 	uint32_t * output_image;
// 	// uint32_t * output_image = malloc(sizeof(uint32_t) * num_frames);
// 	uint16_t * output_palette = malloc(sizeof(uint16_t) * 16);
// 	png_details_t p_det;

// 	//malloc enough space for "num_frames" amount of uint32_t pointers. Have a single png_details struct at a time
// 	if(num_frames == 1){
// 		//If the dimension of any of the pngs isn't 32-by-32, then call invalid_input();
// 	}
// 	else{
// 		;
// 	}

// 	// 1, 0, 1, 4, 7, 0, 2

// 	free(output_image);
// 	free(output_palette);
// 	return 0;
// }

uint8_t png_to_argb4444_4BPP(png_details_t * p_det, uint32_t * image_buffer, uint16_t * palette_buffer){
	;
}

int main(int argC, char * argV[]){
	bool flag_input_image = false;
	bool flag_output_image = false;
	bool flag_output_palette = false;
	uint8_t input_image_index_start = 0;
	uint8_t input_image_index_end = 0;
	uint8_t output_image_index = 0;
	uint8_t output_palette_index = 0;
	for(int i = 1; i < argC; i++){
		if(string_equals(argV[i], "--input-image")){
			if(++i >= argC){
				invalid_input();
			}
			flag_input_image = true;
			input_image_index_start = i;
			input_image_index_end = input_image_index_start;
			while(i + 1 < argC && !string_equals(argV[i + 1], "--output-image") && !string_equals(argV[i + 1], "--output-palette")){
				i++;
				input_image_index_end = i;
			}
		}
		else if(string_equals(argV[i], "--output-image")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output_image = true;
			output_image_index = i;
		}
		else if(string_equals(argV[i], "--output-palette")){
			if(++i >= argC){
				invalid_input();
			}
			flag_output_palette = true;
			output_palette_index = i;
		}
	}

	//A file path is not set, can't continue
	if(!flag_input_image || !flag_output_image || !flag_output_palette){
		invalid_input();
	}

	uint8_t num_frames = input_image_index_end - input_image_index_start + 1;

	uint32_t * output_image;
	uint16_t * output_palette = malloc(sizeof(uint16_t) * 16);
	if(!output_palette){printf("Ran out of memory. Terminating now\n"); return 1;}
	png_details_t p_det;

	//malloc enough space for "num_frames" amount of uint32_t pointers. Have a single png_details struct at a time
	if(num_frames == 1){
		if(read_png_file(argV[input_image_index_start], &p_det)){printf("File %s couldn't be read. Terminating now\n", argV[input_image_index_start]); return 2;}
		if(p_det.width != 32 || p_det.height % 32 != 0){
			printf("%s has incorrect height/width\nWidth should be 32 and height a multiple of 32\nThis image's width is %d and height is %d\n",
				argV[input_image_index_start], p_det.width, p_det.height);
			return 3;
		}
		else{
			printf("Correct dimentions. Width %d Height %d\n", p_det.width, p_det.height);
		}

		output_image = malloc(sizeof(uint32_t) * num_frames * p_det.height);	//The uint32_t size acts as the width
		if(!output_image){printf("Ran out of memory. Terminating now\n"); return 4;}

		free_png_texture_buffer(&p_det);
	}
	else{
		printf("Multiple sources isn't supported yet\n");
		return 1;
	}

	free(output_image);
	free(output_palette);
	return 0;







	//malloc enough space for "num_frames" amount of uint32_t pointers. Have a single png_details struct at a time
	// if(num_frames != 1){
		//If the dimension of any of the pngs isn't 32-by-32, then call invalid_input();
	// }

	// printf("%d, %d, %d, %d, %d, %d, %d\n", flag_input_image, flag_output_image, flag_output_palette,
	// 	input_image_index_start, input_image_index_end, output_image_index, output_palette_index);

	// ./VmuSavefileIconCreator --output-palette pal.bin --input-image a.png b.png c.png d.png --output-image image.bin
	// 1, 1, 1, 4, 7, 9, 2

	// ./VmuSavefileIconCreator --output-palette pal.bin --input-image a.png b.png c.png d.png
	// 1, 0, 1, 4, 7, 0, 2

	return 0;
}
