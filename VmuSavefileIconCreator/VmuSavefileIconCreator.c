#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "png_assist.h"


void invalid_input(){
	// printf("\nWrong number of arguments provided. This is the format\n");
	// printf("./DtexToRGBA8888 [dtex_filename] *[--binary] [rgba8888_binary_filename] *[--png] [png_filename]\n");
	// printf("png and binary is an optional tag that toggle png and/or binary outputs\n");
	// printf("However you must choose at least one\n");

	// printf("Please not the following stuff isn't supported:\n");
	// printf("\t-BUMPMAP pixel format\n");
	// printf("\t-Compressed\n");
	// printf("\t-Mipmapped\n");

	return;
}

int main(int argC, char *argV[]){
	// bool flag_binary_preview = false;
	// uint8_t binary_index = 0;
	// bool flag_png_preview = false;
	// uint8_t png_index = 0;
	// for(int i = 1; i < argC; i++){
	// 	//1st param is reserved for the dtex name
	// 	if(i == 1 && !(strlen(argV[i]) >= 4 && strcmp(argV[i] + strlen(argV[i]) - 5, ".dtex") == 0)){
	// 		invalid_input();
	// 		return 1;
	// 	}

	// 	if(!strcmp(argV[i], "--binary")){
	// 		if(i + 1 >= argC){
	// 			invalid_input();
	// 			return 1;
	// 		}
	// 		flag_binary_preview = true;
	// 		binary_index = i + 1;
	// 	}

	// 	if(!strcmp(argV[i], "--png")){
	// 		if(i + 1 < argC && strlen(argV[i + 1]) >= 4 && strcmp(argV[i + 1] + strlen(argV[i + 1]) - 4, ".png") == 0){
	// 			flag_png_preview = true;
	// 			png_index = i + 1;
	// 		}
	// 		else{
	// 			invalid_input();
	// 			return 1;
	// 		}
	// 	}
	// }

	// if(!flag_binary_preview && !flag_png_preview){
	// 	invalid_input();
	// 	return 1;
	// }

	// uint32_t * texture = NULL;
	// uint16_t height = 0;
	// uint16_t width = 0;

	// uint8_t error_code = load_dtex(argV[1], &texture, &height, &width);
	// if(error_code){
	// 	printf("Error %d has occurred. Terminating program\n", error_code);
	// 	free(texture);
	// 	return 1;
	// }

	// //Output a binary if requested
	// if(flag_binary_preview){
	// 	FILE * f_binary = fopen(argV[binary_index], "wb");
	// 	if(f_binary == NULL){
	// 		printf("Error opening file!\n");
	// 		return 1;
	// 	}
	// 	fwrite(texture, sizeof(uint32_t), height * width, f_binary);	//Note this is stored in little endian
	// 	fclose(f_binary);
	// }

	// //Output a PNG if requested
	// if(flag_png_preview){
	// 	png_details_t p_det;
	// 	if(rgba8888_to_png_details(texture, height, width, &p_det)){return 1;}
	// 	write_png_file(argV[png_index], &p_det);
	// }
	// free(texture);

	return 0;
}
