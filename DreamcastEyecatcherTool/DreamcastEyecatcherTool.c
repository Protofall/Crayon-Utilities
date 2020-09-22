#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>	//For rand
#include <math.h>	//pow, sqrt

#include "png_assist.h"


void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./DreamcastEyecatcherTool --input-image [png_filename] --output-binary [filename] --type [1 - 3] *--preview [filename]\n\n");
	printf("Note you can only have 1 png file\nThe PNG must be 72 pixels wide and 56 pixels high\n");
	printf("Mode must be there with a value of 1, 2 or 3.\n");
	printf("\t1 is a raw argb4444 binary (15.75 Blocks)\n");
	printf("\t2 is a PAL8BPP argb4444 binary allowing 256 colours (8.875 Blocks)\n");
	printf("\t3 is a PAL4BPP argb4444 binary allowing 16 colours (4 Blocks)\n");
	printf("Preview is optional and just gives you a preview of the binary\n");

	exit(1);
}

//Just to make it more obvious to people viewing the code what is happening
bool string_equals(char * a, char * b){
	return !strcmp(a, b);
}

//Takes the png and makes a uint32_t binary
uint8_t png_to_rgba8888(uint8_t in_img_index, uint32_t ** input_image, uint16_t * width,
	uint16_t * height, char ** argV){

	uint16_t buff_width = 72;
	uint16_t buff_height = 56;
	*input_image = malloc(sizeof(uint32_t) * buff_width * buff_height);
	if(!(*input_image)){printf("Ran out of memory. Terminating now\n"); return 1;}
	uint32_t * current_texel = *input_image;
	png_details_t p_det;

	if(read_png_file(argV[in_img_index], &p_det)){printf("File %s couldn't be read. Terminating now\n", argV[in_img_index]); return 2;}
	if(p_det.width != buff_width){
		printf("%s has incorrect width\nWidth should be 72\nThis image's width is %d\n",
			argV[in_img_index], p_det.width);
		return 3;
	}
	if(p_det.height != buff_height){
		printf("%s has incorrect height\nHeight should be 56\nThis image's height is %d\n",
			argV[in_img_index], p_det.height);
		return 3;
	}
	*height = p_det.height;
	*width = p_det.width;

	for(int y = 0; y < p_det.height; y++){
		for(int x = 0; x < p_det.width; x++){
			png_bytep px = &(p_det.row_pointers[y][x * 4]);
			current_texel[0] = (px[0] << 24) + (px[1] << 16) + (px[2] << 8) + px[3];
			// memcpy(&current_texel[0], &p_det.row_pointers[y][x * 4], 4);	//Byte order is wrong when using this method
			current_texel++;
		}
	}

	free_png_texture_buffer(&p_det);

	return 0;
}

//This will also help reduce the colour count by removing similar colours
void rgba8888_to_argb4444(uint32_t * input_image, uint16_t width, uint16_t height){
	uint8_t r, g, b, a;
	for(int i = 0; i < width * height; i++){
		r = bit_extracted(input_image[i], 8, 24) >> 4;
		g = bit_extracted(input_image[i], 8, 16) >> 4;
		b = bit_extracted(input_image[i], 8, 8) >> 4;
		a = bit_extracted(input_image[i], 8, 0) >> 4;
		input_image[i] = (a << 12) + (r << 8) + (g << 4) + b;	//ARGB4444

	}
	return;
}

typedef struct colour_entry{
	uint32_t colour;
	struct colour_entry * next;
} colour_entry_t;

uint8_t k_means(uint16_t * output_palette, uint16_t k, colour_entry_t * full_palette_head){
	//We re-calculate colour_count here instead of param incase a user provided a wrong size
	colour_entry_t * current = full_palette_head;
	uint32_t colour_count = 0;
	while(current != NULL){
		colour_count++;
		current = current->next;
	}
	if(k >= colour_count){	//No need to reduce colours then
		return 0;
	}

	//Generate k random colours and store in "output_palette"
	for(int i = 0; i < k; i++){
		output_palette[i] = (rand() % (1 << 16));	//1 << 16 = 65336 = 2 ^ 16
		for(int j = 0; j < i; j++){
			if(output_palette[j] == output_palette[i]){	//This exact colour already exists, choose a new colour
				i--;
				break;
			}
		}
	}
	
	uint8_t error_code = 0;
	double * distance_array = NULL;
	uint32_t * owned_node_count = NULL;
	uint16_t * sum_nearest_A = NULL;
	uint16_t * sum_nearest_R = NULL;
	uint16_t * sum_nearest_G = NULL;
	uint16_t * sum_nearest_B = NULL;
	uint16_t * new_centroids = NULL;

	//We only calculate for one datapoint colour at a time to see which centroid that node is closest to
	distance_array = malloc(sizeof(double) * k);
	if(!distance_array){
		printf("Ran out of memory. Terminating now\n");
		error_code = 1; goto cleanup_k_means;
	}
	//And this array tells us how many nodes are closest to a each centroid. Same as old "closePoint"
	owned_node_count = malloc(sizeof(uint32_t) * k);
	if(!owned_node_count){
		printf("Ran out of memory. Terminating now\n");
		error_code = 2; goto cleanup_k_means;
	}

	//Due to overflow I'm splitting each channel into its own thing :/
		//A 16-bit var should be enough space to avoid overflow
	sum_nearest_A = malloc(sizeof(uint16_t) * k);
	if(!sum_nearest_A){
		printf("Ran out of memory. Terminating now\n");
		error_code = 3; goto cleanup_k_means;
	}
	sum_nearest_R = malloc(sizeof(uint16_t) * k);
	if(!sum_nearest_R){
		printf("Ran out of memory. Terminating now\n");
		error_code = 4; goto cleanup_k_means;
	}
	sum_nearest_G = malloc(sizeof(uint16_t) * k);
	if(!sum_nearest_G){
		printf("Ran out of memory. Terminating now\n");
		error_code = 5; goto cleanup_k_means;
	}
	sum_nearest_B = malloc(sizeof(uint16_t) * k);
	if(!sum_nearest_B){
		printf("Ran out of memory. Terminating now\n");
		error_code = 6; goto cleanup_k_means;
	}
	new_centroids = malloc(sizeof(uint16_t) * k);
	if(!new_centroids){
		printf("Ran out of memory. Terminating now\n");
		error_code = 7; goto cleanup_k_means;
	}

	//Somewhere below here there must be a memory leak...how???
	double px[4];
	int nearest;	//Default zero
	int same;		//Used to check if the centroids have changed once updated
	while(1){
		//Reset the arrays for a new loop
		for(int i = 0; i < k; i++){
			sum_nearest_A[i] = 0;
			sum_nearest_R[i] = 0;
			sum_nearest_G[i] = 0;
			sum_nearest_B[i] = 0;
			new_centroids[i] = 0;
			owned_node_count[i] = 0;
		}

		//Calculates the distance between points and centroids
		current = full_palette_head;
		while(current != NULL){	//Goes "colour_count" times
			nearest = 0;
			for(int i = 0; i < k; i++){
				px[0] = pow(bit_extracted(current->colour, 4, 12) - bit_extracted(output_palette[i], 4, 12), 2);
				px[1] = pow(bit_extracted(current->colour, 4, 8) - bit_extracted(output_palette[i], 4, 8), 2);
				px[2] = pow(bit_extracted(current->colour, 4, 4) - bit_extracted(output_palette[i], 4, 4), 2);
				px[3] = pow(bit_extracted(current->colour, 4, 0) - bit_extracted(output_palette[i], 4, 0), 2);
				distance_array[i] = sqrt(px[0] + px[1] + px[2] + px[3]);
				if(distance_array[i] < distance_array[nearest]){
					nearest = i;
				}
			}
			owned_node_count[nearest]++;	//The node in the i loop belongs to the "nearest" centroid
			sum_nearest_A[nearest] += bit_extracted(current->colour, 4, 12);
			sum_nearest_R[nearest] += bit_extracted(current->colour, 4, 8);
			sum_nearest_G[nearest] += bit_extracted(current->colour, 4, 4);
			sum_nearest_B[nearest] += bit_extracted(current->colour, 4, 0);
			current = current->next;
		}

		//Calculate the final value for the new centroids
		for(int i = 0; i < k; i++){
			uint32_t local_owned_count = owned_node_count[i];
			if(local_owned_count == 0){	//Avoid divide by zero errors
				continue;
			}
			new_centroids[i] = (sum_nearest_A[i] / owned_node_count[i]) << 12;
			new_centroids[i] += (sum_nearest_R[i] / owned_node_count[i]) << 8;
			new_centroids[i] += (sum_nearest_G[i] / owned_node_count[i]) << 4;
			new_centroids[i] += (sum_nearest_B[i] / owned_node_count[i]) << 0;
		}

		//Checking if the centroids are the same as they were before
		same = 0;
		for(int i = 0; i < k; i++){
			if(new_centroids[i] == output_palette[i]){	//If the centroid hasn't changed
				same++;
			}
			output_palette[i] = new_centroids[i];
		}
		if(k <= same){	//If none of the centroids changed, break from the while loop
			break;
		}
	}

	cleanup_k_means:

	free(distance_array);
	free(owned_node_count);
	free(sum_nearest_A);
	free(sum_nearest_R);
	free(sum_nearest_G);
	free(sum_nearest_B);
	free(new_centroids);

	return error_code;
}

//This almost never gets near 16 colours. Best I've seen is 10...
int8_t reduce_colours(uint32_t * texture, uint16_t width, uint16_t height, uint16_t * output_palette,
	uint16_t colour_limit){
	//Get a linked list of all colours present
	colour_entry_t * head = NULL;
	colour_entry_t * tail = NULL;
	colour_entry_t * current = NULL;
	uint16_t * palette_comparison = NULL;
	uint32_t colour_count = 0;
	int8_t return_value = 0;
	for(uint32_t i = 0; i < width * height; i++){
		bool colour_found = false;
		current = head;
		while(current != NULL){
			if(current->colour == texture[i]){
				colour_found = true;
				break;
			}
			current = current->next;
		}
		if(!colour_found){
			colour_entry_t * new_colour = malloc(sizeof(colour_entry_t));
			if(!new_colour){
				return_value = 1;
				goto cleanup;
			}
			if(head == NULL){
				head = new_colour;
			}
			if(tail != NULL){
				tail->next = new_colour;
			}
			tail = new_colour;
			new_colour->colour = texture[i];
			new_colour->next = NULL;

			colour_count++;
		}
	}

	printf("%d unique colours were detected\n", colour_count);

	//If we are already under or equal to the colour limit, then there's no reason to
		//Perform K-means. So we create the paletted texture here and return with code -1
	if(colour_count <= colour_limit){
		return_value = -1;
		colour_entry_t * traverse = head;
		uint8_t i = 0;
		while(traverse != NULL){
			output_palette[i] = traverse->colour;
			i++;
			traverse = traverse->next;
		}
		goto cleanup;
	}

	printf("WARNING: More than %d colours between all source images. ", colour_limit);
	printf("Performing colour-reduction process. Result may be undesireable ");
	printf("since the algorithm isn't perfect so I recommend making your own ");
	printf("%d-colour image and then run it through this program again for better results.\n", colour_limit);

	//Perform K-means with "colour_limit" centroids
	//Its basically 3 steps:
		//Step 1 is randomly distribute the centroids (Give them random values)
		//Enter a loop
			//Step 2, have a list saying which centroid each point is closest to. (num centroids * "colour_count length boolean array"s)
			//Step 3 is find the mean for each centroid's given list

		//Eg. Node 1, 3, 4 and 6 are closest to C1. Node 2 and 5 are closest to C2.
			//C1's new position is the mean of nodes 1, 3, 4 and 6 and C2's new pos is mean of Node 2 and 5
			//Note to seperate argb when doing these calculation

	palette_comparison = malloc(sizeof(uint16_t) * colour_count);
	if(!palette_comparison){
		return_value = 2;
		goto cleanup;
	}

	//Attempts to find the 16 best colours, but never gets near that...
	uint8_t k_res = k_means(output_palette, colour_limit, head);
	if(k_res){
		return_value = 2 + k_res;
		goto cleanup;
	}

	//Find distance between each node in the colour_entry_t entry and each palette colour to find the best substitute
	double px[4];
	current = head;
	for(int i = 0; i < colour_count; i++){
		uint32_t nearest_index;	//Always set to zero in first iteration of J loop so never uninitialised
		double distance = -1;
		for(int j = 0; j < colour_limit; j++){
			px[0] = pow(bit_extracted(current->colour, 4, 12) - bit_extracted(output_palette[j], 4, 12), 2);
			px[1] = pow(bit_extracted(current->colour, 4, 8) - bit_extracted(output_palette[j], 4, 8), 2);
			px[2] = pow(bit_extracted(current->colour, 4, 4) - bit_extracted(output_palette[j], 4, 4), 2);
			px[3] = pow(bit_extracted(current->colour, 4, 0) - bit_extracted(output_palette[j], 4, 0), 2);
			double current_distance = sqrt(px[0] + px[1] + px[2] + px[3]);
			if(distance < 0 || current_distance < distance){
				distance = current_distance;
				nearest_index = j;
			}
		}
		palette_comparison[i] = output_palette[nearest_index];
		current = current->next;
	}

	//Modify the colours in the texture itself
	for(int i = 0; i < width * height; i++){
		current = head;
		uint32_t thing = 0;
		int j = 0;
		while(current != NULL){
			if(current->colour == texture[i]){
				texture[i] = palette_comparison[j];
				break;
			}
			current = current->next;
			j++;
		}
	}

	cleanup:

	if(palette_comparison){
		free(palette_comparison);
	}

	//Destory the old linked list
	while(head != NULL){
		colour_entry_t * old = head;
		head = head->next;
		free(old);
	}

	return return_value;
}

//Used for the preview function
uint8_t argb4444_to_png_details(uint32_t * pixel_data, uint16_t width, int height, png_details_t * p_det){
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

			//(2^8 - 1) / (2^4 - 1) = 17
			px[0] = bit_extracted(pixel_data[(y * width) + x], 4, 8) * 17;
			px[1] = bit_extracted(pixel_data[(y * width) + x], 4, 4) * 17;
			px[2] = bit_extracted(pixel_data[(y * width) + x], 4, 0) * 17;
			px[3] = bit_extracted(pixel_data[(y * width) + x], 4, 12) * 17;
		}
	}
	return 0;
}

int16_t get_index(uint32_t colour, uint16_t * palette, uint16_t length){
	for(int i = 0; i < length; i++){
		if(colour == palette[i]){
			return i;
		}
	}
	return -1;
}

uint8_t create_binary(uint32_t * input_image, uint16_t * palette, uint8_t ** output_image, uint16_t width,
	uint16_t height, uint8_t mode){
	int output_index = 0;
	if(mode == 1){
		*output_image = malloc(sizeof(uint8_t) * width * height * 2);				//Not paletted
		if(!(*output_image)){printf("Ran out of memory. Terminating now\n"); return 1;}
		for(int i = 0; i < width * height; i++){
			(*output_image)[output_index++] = bit_extracted(input_image[i], 8, 0);
			(*output_image)[output_index++] = bit_extracted(input_image[i], 8, 8);
		}
	}
	else if(mode == 2){
		*output_image = malloc((sizeof(uint8_t) * width * height) + (256 * 2));		//8BPP
		if(!(*output_image)){printf("Ran out of memory. Terminating now\n"); return 1;}
		for(int i = 0; i < 256; i++){
			(*output_image)[output_index++] = bit_extracted(palette[i], 8, 0);
			(*output_image)[output_index++] = bit_extracted(palette[i], 8, 8);
		}
		for(int i = 0; i < width * height; i++){
			(*output_image)[output_index++] = get_index(input_image[i], palette, 256);
		}
	}
	else if(mode == 3){
		*output_image = malloc((sizeof(uint8_t) * width * height / 2) + (16 * 2));	//4BPP
		if(!(*output_image)){printf("Ran out of memory. Terminating now\n"); return 1;}
		for(int i = 0; i < 16; i++){
			(*output_image)[output_index++] = bit_extracted(palette[i], 8, 0);
			(*output_image)[output_index++] = bit_extracted(palette[i], 8, 8);
		}
		for(int i = 0; i < width * height; i++){
			(*output_image)[output_index] = get_index(input_image[i], palette, 16) << 4;
			i++;
			(*output_image)[output_index++] += get_index(input_image[i], palette, 16);
		}
	}

	return 0;
}

uint8_t write_to_file(char * path, size_t size, uint32_t num_elements, void * buffer){
	FILE * f_binary = fopen(path, "wb");
	if(!f_binary){
		printf("Error opening file!\n");
		return 1;
	}
	if(fwrite(buffer, size, num_elements, f_binary) != num_elements){	//Note this is stored in little endian. Also crashes when I try to print 16 entries...
		printf("Error occurred when writing to file\n");
		return 2;
	}
	fclose(f_binary);
	return 0;
}

int main(int argC, char ** argV){
	bool flag_input_image = false;
	bool flag_output_image = false;
	bool flag_mode = false;
	bool flag_preview = false;
	uint8_t input_image_index = 0;
	uint8_t output_image_index = 0;
	uint8_t mode = 0;
	uint8_t preview_index = 0;
	for(int i = 1; i < argC; i++){
		if(string_equals(argV[i], "--input-image") || string_equals(argV[i], "-i")){
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
			flag_output_image = true;
			output_image_index = i;
		}
		else if(string_equals(argV[i], "--type") || string_equals(argV[i], "--mode") ||
			string_equals(argV[i], "-t") || string_equals(argV[i], "-m")){
			if(++i >= argC){
				invalid_input();
			}
			flag_mode = true;
			mode = atoi(argV[i]);
		}
		else if(string_equals(argV[i], "--preview") || string_equals(argV[i], "-p")){
			if(++i >= argC){
				invalid_input();
			}
			flag_preview = true;
			preview_index = i;
		}
	}

	//A file path is not set, can't continue
	if(!flag_input_image || !flag_output_image || !flag_mode){
		invalid_input();
	}

	uint32_t * input_image = NULL;
	uint16_t height = 0;
	uint16_t width = 0;
	uint8_t * output_image = NULL;	//4BPP, Each element contains two texels
	uint16_t * output_palette = NULL;
	uint16_t total_colours;
	uint16_t binary_size;
	if(mode == 1){
		total_colours = 0;
		binary_size = 8064;
	}
	if(mode == 2){
		total_colours = 256;
		binary_size = (total_colours * 2) + 4032;
	}
	else if(mode == 3){
		total_colours = 16;
		binary_size = (total_colours * 2) + 2016;
	}
	output_palette = malloc(sizeof(uint16_t) * total_colours);
	for(int i = 0; i < total_colours; i++){	//Not needed, but is good practice
		output_palette[i] = 0;
	}

	uint8_t error = png_to_rgba8888(input_image_index, &input_image,
		&width, &height, argV);
	if(error){
		printf("Error occurred\n");
		free(input_image);
		return error;
	}

	rgba8888_to_argb4444(input_image, width, height);
	srand(time(NULL));
	uint16_t colour_count;
	int8_t reduce_val = 0;
	//Everything before this point looks good...not sure about beyond
	if(mode == 2){	//PAL8BPP
		reduce_val = reduce_colours(input_image, width, height, output_palette, 256);
	}
	else if(mode == 3){	//PAL4BPP
		reduce_val = reduce_colours(input_image, width, height, output_palette, 16);
	}
	if(reduce_val > 0){
		printf("Ran out of memory. Terminating now\n");
		free(input_image);
		free(output_image);
		return 4;
	}
	if(reduce_val == -1){
		printf("Colours didn't need to be reduced\n");
	}

	if(flag_preview){
		png_details_t p_det;
		if(argb4444_to_png_details(input_image, width, height, &p_det)){return 5;}
		write_png_file(argV[preview_index], &p_det);
	}

	error = create_binary(input_image, output_palette, &output_image, width, height, mode);
	free(input_image);	//This buffer is no longer needed
	free(output_palette);
	if(error){
		printf("Error occurred\n");
		free(output_image);
		return 5 + error;
	}

	error = write_to_file(argV[output_image_index], sizeof(uint8_t), binary_size, output_image);
	if(error){
		free(output_image);
		return 6 + error;	//Goes to 8
	}

	free(output_image);
	return 0;
}
