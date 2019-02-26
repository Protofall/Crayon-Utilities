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

//Takes all png's listed and makes one long binary. I always store it in a 1MB buffer
	//because 32 * 32 * 256 ("Max frames") * 4 (Bytes in RGBA8888) = 1MB
	//Do note that you would never use this whole buffer because the VMUs only have
	//200 Blocks of data (512 Bytes per block) and one frame of an icon is 1 Block
	//and also due to how I take input, the input_image start/end difference can only go
	//up to 253 frames. I don't see this is a problem because why would you want this many frames
	//and a 1MB buffer is nothing to modern PC running this program so its fine to over allocate.
uint8_t pngs_to_one_binary(uint8_t in_img_index_start, uint8_t in_img_index_end, uint8_t output_image_index,
	uint8_t output_palette_index, uint32_t ** input_image, uint16_t * height, char ** argV){

	*input_image = malloc(sizeof(uint32_t) * 32 * 32 * 256);
	uint32_t * current_image = *input_image;
	if(!(*input_image)){printf("Ran out of memory. Terminating now\n"); return 1;}
	png_details_t p_det;

	uint8_t num_frames = in_img_index_end - in_img_index_start + 1;
	// uint16_t height = 0;

	for(int i = 0; i < num_frames; i++){
		printf("File is %s\n", argV[in_img_index_start + i]);
		if(read_png_file(argV[in_img_index_start + i], &p_det)){printf("File %s couldn't be read. Terminating now\n", argV[in_img_index_start + i]); return 2;}
		if(p_det.width != 32 || p_det.height % 32 != 0){
			printf("%s has incorrect height/width\nWidth should be 32 and height a multiple of 32\nThis image's width is %d and height is %d\n",
				argV[in_img_index_start], p_det.width, p_det.height);
			return 3;
		}
		*height += p_det.height;
		if(*height > 256 * 32){
			printf("Combined PNG height too long for buffer. Terminating program\n");
			return 3;
		}

		for(int y = 0; y < p_det.height; y++){
			for(int x = 0; x < p_det.width; x++){
				png_bytep px = &(p_det.row_pointers[y][x * 4]);
				current_image[0] = (px[0] << 24) + (px[1] << 16) + (px[2] << 8) + px[3];
				// memcpy(&current_image[0], &p_det.row_pointers[y][x * 4], 4);	//Byte order is wrong when using this method
				current_image++;
			}
		}

		free_png_texture_buffer(&p_det);
	}

	//This was used for debugging
	if(rgba8888_to_png_details(*input_image, *height, 32, &p_det)){return 190;}
	write_png_file("test.png", &p_det);	//Image looks all messed up for some reason

	return 0;
}

//Is a little error-ous
bool texture_within_16_colours(uint32_t * texture, uint16_t height, uint16_t ** palette_processed){
	uint32_t palette[16];
	uint8_t palette_index = 0;
	for(int i = 0; i < 32 * height; i++){
		bool found = false;
		for(int j = 0; j < palette_index; j++){
			if(texture[i] == palette[j]){
				found = true;
				break;
			}
		}
		if(!found){
			if(palette_index >= 16){
				return false;
			}
			palette[palette_index];
			palette_index++;
		}
	}
	printf("Total colours: %d\n", palette_index);
	return true;
}

// uint8_t png_to_argb4444_4BPP(png_details_t * p_det, uint32_t * image_buffer, uint16_t * palette_buffer){
// 	;
// }

int main(int argC, char ** argV){
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

	// uint8_t num_frames = input_image_index_end - input_image_index_start + 1;

	uint32_t * input_image = NULL;
	uint16_t height = 0;
	uint8_t * output_image = NULL;	//4BPP, Each element contains two texels
	uint16_t * output_palette = NULL;
	// uint16_t * output_palette = malloc(sizeof(uint16_t) * 16);
	// if(!output_palette){printf("Ran out of memory. Terminating now\n"); return 1;}

	uint8_t error = 0;
	if(error = pngs_to_one_binary(input_image_index_start, input_image_index_end, output_image_index, output_palette_index, &input_image, &height, argV) != 0){
		printf("Error occurred\n");
		free(input_image);
		return error;
	}

	bool is_4BPP = texture_within_16_colours(input_image, height, &output_palette);
	if(is_4BPP){
		printf("It is good\n");
	}
	else{
		printf("More than 16 colours\n");
	}


	free(input_image);
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
