#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "png_assist.h"


void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./VmuSfIconCreator --input-image [png_filename_1] [png_filename_2] (etc.) --output-image [filename] -- output-palette [filename]\n\n");
	printf("Note you ned at least 1 png file\nSuccessing PNGs will be vertically appended in chronological order\n");
	printf("All source PNGs must be 32 pixels wide and be \"A multiple of 32\" pixels height\n");

	printf("\nNote: Right now the program only supports a total of 16 colours between all PNGs\n");
	printf("Any more and the program throws an error\n");
	printf("A future update will see this and find the best 16 colours to use\n");

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
	if(!(*input_image)){printf("Ran out of memory. Terminating now\n"); return 1;}
	uint32_t * current_texel = *input_image;
	png_details_t p_det;

	uint8_t num_pngs = in_img_index_end - in_img_index_start + 1;

	for(int i = 0; i < num_pngs; i++){
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
				current_texel[0] = (px[0] << 24) + (px[1] << 16) + (px[2] << 8) + px[3];
				// memcpy(&current_texel[0], &p_det.row_pointers[y][x * 4], 4);	//Byte order is wrong when using this method
				current_texel++;
			}
		}

		free_png_texture_buffer(&p_det);
	}

	//This was used for debugging
	if(rgba8888_to_png_details(*input_image, *height, 32, &p_det)){return 190;}
	write_png_file("test.png", &p_det);	//Image looks all messed up for some reason

	return 0;
}

//This will also help reduce the colour count by removing similar colours
void rgba8888_to_argb4444(uint32_t * input_image, uint16_t height){
	uint8_t r, g, b, a;
	for(int i = 0; i < 32 * height; i++){
		r = bit_extracted(input_image[i], 8, 24) >> 4;
		g = bit_extracted(input_image[i], 8, 16) >> 4;
		b = bit_extracted(input_image[i], 8, 8) >> 4;
		a = bit_extracted(input_image[i], 8, 0) >> 4;
		input_image[i] = (a << 12) + (r << 8) + (g << 4) + b;
	}
	return;
}

//I think later this function will die off
	//Note that its passed through a n rgba8888_to_argb4444 convertor
bool texture_within_16_colours(uint32_t * texture, uint16_t height, uint16_t * output_palette){
	// uint32_t palette[16];
	uint8_t palette_index = 0;
	for(int i = 0; i < 32 * height; i++){
		bool found = false;
		for(int j = 0; j < palette_index; j++){
			if(texture[i] == output_palette[j]){
				found = true;
				break;
			}
		}
		if(!found){
			printf("%d, %d, %x\n", palette_index, i, texture[i]);
			if(palette_index >= 16){
				return false;
			}
			output_palette[palette_index] = texture[i];
			palette_index++;
		}
	}
	printf("Total colours: %d\n", palette_index);

	//Just to make sure there's no garbage info
	for(int i = palette_index; i < 16; i++){
		output_palette[i] = 0;
		printf("%d, BLANK, %x\n", palette_index, output_palette[i]);
	}

	// uint8_t r, g, b, a;
	// for(int i = 0; i < 16; i++){
	// 	r = bit_extracted(palette[i], 8, 24) >> 4;
	// 	g = bit_extracted(palette[i], 8, 16) >> 4;
	// 	b = bit_extracted(palette[i], 8, 8) >> 4;
	// 	a = bit_extracted(palette[i], 8, 0) >> 4;
	// 	output_palette[i] = (a << 12) + (r << 8) + (g << 4) + b;
	// 	printf("%d, %x\n", i, output_palette[i]);
	// }

	return true;
}

uint8_t create_binaries(uint32_t * input_image, uint8_t ** output_image, uint16_t * output_palette, uint16_t height){
	*output_image = malloc(sizeof(uint8_t) * 32 * height / 2);	//4BPP
	if(!(*output_image)){printf("Ran out of memory. Terminating now\n"); return 1;}

	int output_index = 0;
	for(int i = 0; i < 32 * height; i++){
		for(int j = 0; j < 16; j++){
			if(input_image[i] == output_palette[j]){
				if(i % 2 == 0){
					(*output_image)[output_index] = j << 4;
				}
				else{
					(*output_image)[output_index] += j;
				}
			}
		}
		if(i % 2 != 0){
			output_index++;
		}
	}

	return 0;
}


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

	uint32_t * input_image = NULL;
	uint16_t height = 0;
	uint8_t * output_image = NULL;	//4BPP, Each element contains two texels
	uint16_t output_palette[16];

	uint8_t error = pngs_to_one_binary(input_image_index_start, input_image_index_end, output_image_index,
		output_palette_index, &input_image, &height, argV);
	if(error){
		printf("Error occurred\n");
		free(input_image);
		return error;
	}

	rgba8888_to_argb4444(input_image, height);
	bool is_4BPP = texture_within_16_colours(input_image, height, output_palette);
	if(!is_4BPP){
		printf("More than 16 colours isn't supported yet\n");
		free(input_image);
		return 1;
	}

	error = create_binaries(input_image, &output_image, output_palette, height);
	if(error){
		printf("Error occurred\n");
		free(input_image);
		free(output_image);
		return error;
	}
	free(input_image);	//This buffer is no longer needed

	FILE * f_binary = fopen(argV[output_image_index], "wb");
	if(!f_binary){
		printf("Error opening file!\n");
		free(output_image);
		return 1;
	}
	// fwrite(output_image, sizeof(uint8_t), 32 * height / 2, f_binary);	//Note this is stored in little endian
	if(fwrite(output_image, sizeof(uint8_t), 32 * height / 2, f_binary) != 32 * height / 2){	//Note this is stored in little endian. Also crashes when I try to print 16 entries...
		printf("Error occurred when writing to file\n");
		free(output_image);
		return 1;
	}
	fclose(f_binary);

	f_binary = fopen(argV[output_palette_index], "wb");
	if(!f_binary){
		printf("Error opening file!\n");
		free(output_image);
		return 1;
	}
	if(fwrite(output_palette, sizeof(uint16_t), 16, f_binary) != 16){	//Note this is stored in little endian. Also crashes when I try to print 16 entries...
		printf("Error occurred when writing to file\n");
		free(output_image);
		return 1;
	}
	fclose(f_binary);

	free(output_image);
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
