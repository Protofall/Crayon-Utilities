#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

#include "png_assist.h"

#define DTEX_DEBUG 0

typedef struct dtex_header{
	uint8_t magic[4]; //magic number "DTEX"
	uint16_t   width; //texture width in pixels
	uint16_t  height; //texture height in pixels
	uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
	uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
	uint8_t     magic[4]; //magic number "DPAL"
	uint32_t color_count; //number of 32-bit ARGB palette entries
} dpal_header_t;

//This function takes an element from the texture array and converts it from whatever bpc combo you set to RGBA8888
void dtex_argb_to_rgba8888(dtex_header_t * dtex_header, uint32_t * dtex_buffer,
	uint8_t bpc_a, uint8_t bpc_r, uint8_t bpc_g, uint8_t bpc_b){
	uint16_t extracted;

	for(int i = 0; i < dtex_header->width * dtex_header->height; i++){
		extracted = dtex_buffer[i];

		// (extracted value) * (Max for 8bpc) / (Max for that bpc) = value
		if(bpc_a != 0){
			dtex_buffer[i] = bit_extracted(extracted, bpc_a, bpc_r + bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_a) - 1);	//Alpha
		}
		else{	//We do this to avoid a "divide by zero" error and we don't care about colour data for transparent pixels
			dtex_buffer[i] = (1 << 8) - 1;
		}

		dtex_buffer[i] += (bit_extracted(extracted, bpc_r, bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_r) - 1)) << 24;		//R
		dtex_buffer[i] += (bit_extracted(extracted, bpc_g, bpc_b) * ((1 << 8) - 1) / ((1 << bpc_g) - 1)) << 16;				//G
		dtex_buffer[i] += (bit_extracted(extracted, bpc_b, 0) * ((1 << 8) - 1) / ((1 << bpc_b) - 1)) << 8;					//B
	}
	return;
}

uint32_t argb8888_to_rgba8888(uint32_t argb8888){
	return (argb8888 << 8) + bit_extracted(argb8888, 8, 24);
}

//Given an array of indexes and a dtex.pal file, it will replace all indexes with the correct colour
uint8_t apply_palette(dtex_header_t * dtex_header, uint32_t * dtex_buffer, char * texture_path){
	char * palette_path = (char *) malloc(sizeof(char) * (strlen(texture_path) + 5));
	if(!palette_path){return 1;}
	strcpy(palette_path, texture_path);
	palette_path[strlen(texture_path)] = '.';
	palette_path[strlen(texture_path) + 1] = 'p';
	palette_path[strlen(texture_path) + 2] = 'a';
	palette_path[strlen(texture_path) + 3] = 'l';
	palette_path[strlen(texture_path) + 4] = '\0';

	dpal_header_t dpal_header;
	uint8_t pal_error = 0;
	uint32_t * palette_buffer = NULL;

	#define PAL_ERROR(n) {pal_error = n; goto PAL_cleanup;}

		FILE * palette_file = fopen(palette_path, "rb");
		free(palette_path);
		if(!palette_file){PAL_ERROR(2);}

		if(fread(&dpal_header, sizeof(dpal_header_t), 1, palette_file) != 1){PAL_ERROR(3);}

		if(memcmp(dpal_header.magic, "DPAL", 4)){PAL_ERROR(4);}

		palette_buffer = (uint32_t *) malloc(sizeof(uint32_t) * dpal_header.color_count);
		if(palette_buffer == NULL){PAL_ERROR(5);}

		if(fread(palette_buffer, sizeof(uint32_t) * dpal_header.color_count, 1, palette_file) != 1){PAL_ERROR(6);}

		for(int i = 0; i < dtex_header->width * dtex_header->height; i++){
			if(dtex_buffer[i] < dpal_header.color_count){
				dtex_buffer[i] = argb8888_to_rgba8888(palette_buffer[dtex_buffer[i]]);
			}
			else{	//Invalid palette index
				printf("Paletted texture (Index %d) references colour ID (%u) that DNE\n", i, dtex_buffer[i]);
				PAL_ERROR(7);
			}
		}

	#undef PAL_ERROR

	PAL_cleanup:

	if(palette_buffer != NULL){
		free(palette_buffer);
	}

	if(palette_file != NULL){
		fclose(palette_file);
	}

	return pal_error;
}

int32_t bound(int32_t min, int32_t val, int32_t max){
	if(val < min){
		return min;
	}
	if(val > max){
		return max;
	}
	return val;
}

//The image appears to be abit dull. Need to check if this is how it should be or the texconv preview isn't accurate
uint32_t yuv444_to_rgba8888(uint8_t y, uint8_t u, uint8_t v){
	//I think this is fine since a uint8_t's range is 0 to 255 and I think a int8_t's range is -128 to 127
	int8_t u2 = u - 128;
	int8_t v2 = v - 128;

	//Note RGB must be clamped within 0 to 255
	//We can't just cast it to a uint8_t because we don't want it to over/underflow
	uint8_t R = bound(0, (int)(y + 1.375 * v2), 255);
	uint8_t G = bound(0, (int)(y - (0.25 * 1.375) * u2 - (0.5 * 1.375 * v2)), 255);
	uint8_t B = bound(0, (int)(y + (1.25 * 1.375) * u2), 255);

	return (R << 24) + (G << 16) + (B << 8) + 255;	//RGBA with full alpha
}

//Note YUV422 doesn't mean 4 + 2 + 2 bits per pixel (1 bytes per pixel), its actually a ration. 4:2:2

//To convert from YUV422 to RGB888, we first need to convert to YUV444

//YUV444    3 bytes per pixel     (12 bytes per 4 pixels)
//YUV422    4 bytes per 2 pixels  ( 8 bytes per 4 pixels)
void yuv422_to_rgba8888(dtex_header_t * dtex_header, uint32_t * dtex_buffer){
	//We handle 2 pixels at a time (4 bytes = 2 pixels, u y0, v, y1)
	uint16_t byte_section[2];	//Contains 2 vars
	for(int i = 0; i < dtex_header->width * dtex_header->height; i += 2){
		uint8_t u = bit_extracted(dtex_buffer[i], 8, 0);
		uint8_t y0 = bit_extracted(dtex_buffer[i], 8, 8);
		uint8_t v = bit_extracted(dtex_buffer[i + 1], 8, 0);
		uint8_t y1 = bit_extracted(dtex_buffer[i + 1], 8, 8);

		dtex_buffer[i] = yuv444_to_rgba8888(y0, u, v);
		dtex_buffer[i + 1] = yuv444_to_rgba8888(y1, u, v);
	}
	return;
}

//This is the function that JamoHTP did. It boggled my mind too much
uint32_t get_twiddled_index(uint16_t w, uint16_t h, uint32_t p){
	uint32_t ddx = 1, ddy = w;
	uint32_t q = 0;

	for(int i = 0; i < 16; i++){
		if(h >>= 1){
			if(p & 1){q |= ddy;}
			p >>= 1;
		}
		ddy <<= 1;
		if(w >>= 1){
			if(p & 1){q |= ddx;}
			p >>= 1;
		}
		ddx <<= 1;
	}

	return q;
}

//Add a seperate buffer for compressed data before its loaded in
uint8_t load_dtex(char * texture_path, uint32_t ** rgba8888_buffer, uint16_t * height, uint16_t * width){
	dtex_header_t dtex_header;

	FILE * texture_file = fopen(texture_path, "rb");
	if(!texture_file){return 1;}

	if(fread(&dtex_header, sizeof(dtex_header_t), 1, texture_file) != 1){fclose(texture_file); return 2;}

	if(memcmp(dtex_header.magic, "DTEX", 4)){fclose(texture_file); return 3;}

	uint8_t stride_setting = bit_extracted(dtex_header.type, 5, 0);
	bool strided = dtex_header.type & (1 << 25);
	bool twiddled = !(dtex_header.type & (1 << 26));	//Note this flag is TRUE if twiddled
	uint8_t pixel_format = bit_extracted(dtex_header.type, 3, 27);
	bool compressed = dtex_header.type & (1 << 30);
	bool mipmapped = dtex_header.type & (1 << 31);

	//'type' contains the various flags and the pixel format packed together:
	// bits 0-4 : Stride setting.
	// 	The width of stride textures is NOT stored in 'width'. To get the actual
	// 	width, multiply the stride setting by 32. The next power of two size up
	// 	from the stride width will be stored in 'width'.
	// bit 25 : Stride flag
	// 	0 = Non-strided
	// 	1 = Strided
	// bit 26 : Untwiddled flag
	// 	0 = Twiddled
	// 	1 = Untwiddled
	// bits 27-29 : Pixel format
	// 	0 = ARGB1555
	// 	1 = RGB565
	// 	2 = ARGB4444
	// 	3 = YUV422
	// 	4 = BUMPMAP
	// 	5 = PAL4BPP
	// 	6 = PAL8BPP
	// bit 30 : Compressed flag
	// 	0 = Uncompressed
	// 	1 = Compressed
	// bit 31 : Mipmapped flag
	// 	0 = No mipmaps
	// 	1 = Mipmapped

	//Note you can only stride: RGB565, ARGB1555, ARGB4444 and YUV422
	//Stride can't be twiddled, compressed or mipmapped

	#if DTEX_DEBUG == 1
	printf("Format details: str_set %d, str %d, twid %d, pix %d, comp %d, mip %d, whole %d\n",
		stride_setting, strided, twiddled, pixel_format, compressed, mipmapped, dtex_header.type);
	#endif

	//Note: If you plan to add support mip-maps, I believe the mip-mapped YUV422 1x1 texture is actually
		//in RGB565 format because YUV422 can only represent pairs of pixels
	if(mipmapped || pixel_format == 4){
		printf("Program incomplete. Unsupported dtex parameter detected. Stopping conversion\n");
		fclose(texture_file);
		return 4;
	}

	uint8_t bpc[4];
	uint8_t bpp;
	bool rgb = false; bool paletted = false; bool yuv = false; bool bumpmap = false;
	switch(pixel_format){
		case 0:
			bpc[0] = 1; bpc[1] = 5; bpc[2] = 5; bpc[3] = 5; bpp = 16; rgb = true; break;
		case 1:
			bpc[0] = 0; bpc[1] = 5; bpc[2] = 6; bpc[3] = 5; bpp = 16; rgb = true; break;
		case 2:
			bpc[0] = 4; bpc[1] = 4; bpc[2] = 4; bpc[3] = 4; bpp = 16; rgb = true; break;
		case 3:
			bpp = 16; yuv = true; break;
		case 4:
			bpp = 16; bumpmap = true; break;
		case 5:
			bpp = 4; paletted = true; break;
		case 6:
			bpp = 8; paletted = true; break;
		default:
			printf("Pixel format %d doesn't exist\n", pixel_format);
			fclose(texture_file);
			return 5;
	}

	size_t read_size;
	if(bpp == 16){	//ARGB, YUV422, BUMPMAP
		read_size = sizeof(uint16_t);
	}
	else{	//PAL[X]BPP where X == 4 || 8
		read_size = sizeof(uint8_t);
	}

	//Acording to the texconv github, these are the only invalid setting combinations
	if((strided && (paletted || bumpmap)) || (strided && (compressed || mipmapped || twiddled))){
		printf("Invalid combination of header parameters\n");
		fclose(texture_file);
		return 6;
	}

	//I think rectangular mip-maps are illegal based on a comment Tvspelsfreak made maybe here somewhere
		//http://dcemulation.org/phpBB/viewtopic.php?f=29&t=103369#p1045504
	if(mipmapped && (dtex_header.width != dtex_header.height)){
		printf("Invalid combination of header parameters\n");
		fclose(texture_file);
		return 6;
	}

	//The width isn't accurate for strided textures (Goes to next power of two up). This will give us the true width
	if(strided){
		dtex_header.width = stride_setting * 32;
	}

	//When adding compression support, also malloc an array to dump the compressed file
	*rgba8888_buffer = (uint32_t *) malloc(sizeof(uint32_t) * dtex_header.height * dtex_header.width);
	if(*rgba8888_buffer == NULL){
		printf("Malloc failed\n");
		fclose(texture_file);
		return 7;
	}

	//Read the whole file into memory and untwiddle/decompress if need be
	int index;
	if(compressed){
		//The code-book is 2KB (256 lots of 64-bit entries)
		uint32_t * code_book = (uint32_t *) malloc(sizeof(uint32_t) * 2 * 256);	//Each uint32_t pair is one code-book entry
		if(code_book == NULL){
			printf("Malloc failed\n");
			fclose(texture_file);
			return 22;
		}

		uint8_t load_compress_error = 0;
		#define LOAD_COMPRESS_ERROR(n) {load_compress_error = n; goto load_compress_cleanup;}

			if(fread(code_book, sizeof(uint32_t) * 2, 256, texture_file) != 256){	//256 lots of 64-bit entries totals to 2KB
				printf("Dtex file is smaller than expected\n");
				LOAD_COMPRESS_ERROR(21);
			}

			if(bpp == 16){
				uint8_t extracted = 0;
				uint16_t texels[4];
				const int pixels = (dtex_header.width / 2) * (dtex_header.height / 2);
				for(int i = 0; i < pixels; i++){
					if(fread(&extracted, sizeof(uint8_t), 1, texture_file) != 1){LOAD_COMPRESS_ERROR(22);}

					texels[0] = bit_extracted(code_book[(2 * extracted)], 16, 0);
					texels[1] = bit_extracted(code_book[(2 * extracted)], 16, 16);
					texels[2] = bit_extracted(code_book[(2 * extracted) + 1], 16, 0);
					texels[3] = bit_extracted(code_book[(2 * extracted) + 1], 16, 16);
					int index = get_twiddled_index(dtex_header.width / 2, dtex_header.height / 2, i);
					index = ((index % (dtex_header.width / 2)) * 2) + (((index / (dtex_header.width / 2)) * 2) * dtex_header.width);	//Get the true index since
																																		//dims are different
					(*rgba8888_buffer)[index] = texels[0];
					(*rgba8888_buffer)[index + 1] = texels[2];
					(*rgba8888_buffer)[index + dtex_header.width] = texels[1];
					(*rgba8888_buffer)[index + dtex_header.width + 1] = texels[3];
				}
			}
			else if(bpp == 8){
				uint8_t extracted[2];
				uint16_t texels[16];
				const int pixels = (dtex_header.width / 4) * (dtex_header.height / 4);
				for(int i = 0; i < pixels; i++){
					if(fread(&extracted[0], sizeof(uint8_t), 1, texture_file) != 1){LOAD_COMPRESS_ERROR(22);}
					if(fread(&extracted[1], sizeof(uint8_t), 1, texture_file) != 1){LOAD_COMPRESS_ERROR(22);}

					texels[0] = bit_extracted(code_book[(2 * extracted[0])], 8, 0);
					texels[1] = bit_extracted(code_book[(2 * extracted[0])], 8, 8);
					texels[2] = bit_extracted(code_book[(2 * extracted[0])], 8, 16);
					texels[3] = bit_extracted(code_book[(2 * extracted[0])], 8, 24);
					texels[4] = bit_extracted(code_book[(2 * extracted[0]) + 1], 8, 0);
					texels[5] = bit_extracted(code_book[(2 * extracted[0]) + 1], 8, 8);
					texels[6] = bit_extracted(code_book[(2 * extracted[0]) + 1], 8, 16);
					texels[7] = bit_extracted(code_book[(2 * extracted[0]) + 1], 8, 24);
					texels[8] = bit_extracted(code_book[(2 * extracted[1])], 8, 0);
					texels[9] = bit_extracted(code_book[(2 * extracted[1])], 8, 8);
					texels[10] = bit_extracted(code_book[(2 * extracted[1])], 8, 16);
					texels[11] = bit_extracted(code_book[(2 * extracted[1])], 8, 24);
					texels[12] = bit_extracted(code_book[(2 * extracted[1]) + 1], 8, 0);
					texels[13] = bit_extracted(code_book[(2 * extracted[1]) + 1], 8, 8);
					texels[14] = bit_extracted(code_book[(2 * extracted[1]) + 1], 8, 16);
					texels[15] = bit_extracted(code_book[(2 * extracted[1]) + 1], 8, 24);

					int index = get_twiddled_index(dtex_header.width / 4, dtex_header.height / 4, i);
					index = ((index % (dtex_header.width / 4)) * 4) + (((index / (dtex_header.width / 4)) * 4) * dtex_header.width);	//Get the true index since
																																		//dims are different
					(*rgba8888_buffer)[index + 0] = texels[0];
					(*rgba8888_buffer)[index + 1] = texels[2];
					(*rgba8888_buffer)[index + 2] = texels[8];
					(*rgba8888_buffer)[index + 3] = texels[10];
					(*rgba8888_buffer)[index + 0 + dtex_header.width] = texels[1];
					(*rgba8888_buffer)[index + 1 + dtex_header.width] = texels[3];
					(*rgba8888_buffer)[index + 2 + dtex_header.width] = texels[9];
					(*rgba8888_buffer)[index + 3 + dtex_header.width] = texels[11];
					(*rgba8888_buffer)[index + 0 + (2 * dtex_header.width)] = texels[4];
					(*rgba8888_buffer)[index + 1 + (2 * dtex_header.width)] = texels[6];
					(*rgba8888_buffer)[index + 2 + (2 * dtex_header.width)] = texels[12];
					(*rgba8888_buffer)[index + 3 + (2 * dtex_header.width)] = texels[14];
					(*rgba8888_buffer)[index + 0 + (3 * dtex_header.width)] = texels[5];
					(*rgba8888_buffer)[index + 1 + (3 * dtex_header.width)] = texels[7];
					(*rgba8888_buffer)[index + 2 + (3 * dtex_header.width)] = texels[13];
					(*rgba8888_buffer)[index + 3 + (3 * dtex_header.width)] = texels[15];

				}
			}
			else{
				uint8_t extracted = 0;
				uint16_t texels[16];
				const int pixels = (dtex_header.width / 4) * (dtex_header.height / 4);
				for(int i = 0; i < pixels; i++){
					if(fread(&extracted, sizeof(uint8_t), 1, texture_file) != 1){LOAD_COMPRESS_ERROR(22);}

					texels[0] = bit_extracted(code_book[(2 * extracted)], 4, 0);
					texels[1] = bit_extracted(code_book[(2 * extracted)], 4, 4);
					texels[2] = bit_extracted(code_book[(2 * extracted)], 4, 8);
					texels[3] = bit_extracted(code_book[(2 * extracted)], 4, 12);
					texels[4] = bit_extracted(code_book[(2 * extracted)], 4, 16);
					texels[5] = bit_extracted(code_book[(2 * extracted)], 4, 20);
					texels[6] = bit_extracted(code_book[(2 * extracted)], 4, 24);
					texels[7] = bit_extracted(code_book[(2 * extracted)], 4, 28);
					texels[8] = bit_extracted(code_book[(2 * extracted) + 1], 4, 0);
					texels[9] = bit_extracted(code_book[(2 * extracted) + 1], 4, 4);
					texels[10] = bit_extracted(code_book[(2 * extracted) + 1], 4, 8);
					texels[11] = bit_extracted(code_book[(2 * extracted) + 1], 4, 12);
					texels[12] = bit_extracted(code_book[(2 * extracted) + 1], 4, 16);
					texels[13] = bit_extracted(code_book[(2 * extracted) + 1], 4, 20);
					texels[14] = bit_extracted(code_book[(2 * extracted) + 1], 4, 24);
					texels[15] = bit_extracted(code_book[(2 * extracted) + 1], 4, 28);

					int index = get_twiddled_index(dtex_header.width / 4, dtex_header.height / 4, i);
					index = ((index % (dtex_header.width / 4)) * 4) + (((index / (dtex_header.width / 4)) * 4) * dtex_header.width);	//Get the true index since
																																		//dims are different
					(*rgba8888_buffer)[index + 0] = texels[0];
					(*rgba8888_buffer)[index + 1] = texels[2];
					(*rgba8888_buffer)[index + 2] = texels[8];
					(*rgba8888_buffer)[index + 3] = texels[10];
					(*rgba8888_buffer)[index + 0 + dtex_header.width] = texels[1];
					(*rgba8888_buffer)[index + 1 + dtex_header.width] = texels[3];
					(*rgba8888_buffer)[index + 2 + dtex_header.width] = texels[9];
					(*rgba8888_buffer)[index + 3 + dtex_header.width] = texels[11];
					(*rgba8888_buffer)[index + 0 + (2 * dtex_header.width)] = texels[4];
					(*rgba8888_buffer)[index + 1 + (2 * dtex_header.width)] = texels[6];
					(*rgba8888_buffer)[index + 2 + (2 * dtex_header.width)] = texels[12];
					(*rgba8888_buffer)[index + 3 + (2 * dtex_header.width)] = texels[14];
					(*rgba8888_buffer)[index + 0 + (3 * dtex_header.width)] = texels[5];
					(*rgba8888_buffer)[index + 1 + (3 * dtex_header.width)] = texels[7];
					(*rgba8888_buffer)[index + 2 + (3 * dtex_header.width)] = texels[13];
					(*rgba8888_buffer)[index + 3 + (3 * dtex_header.width)] = texels[15];
				}
			}

		#undef LOAD_COMPRESS_ERROR

		load_compress_cleanup:

		fclose(texture_file);
		free(code_book);
		if(load_compress_error){return load_compress_error;}
	}
	else{
		//Had to split PAL8BPP off from 16BPP because sometimes it bugged out when I tried to read a byte into a 16-bit var
		uint8_t load_error = 0;
		#define LOAD_ERROR(n) {load_error = n; goto load_cleanup;}

		if(bpp == 16){
			uint16_t extracted;
			for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
				if(fread(&extracted, read_size, 1, texture_file) != 1){LOAD_ERROR(30);}
				if(twiddled){
					index = get_twiddled_index(dtex_header.width, dtex_header.height, i);	//Given i, it says which element should be there to twiddle it
				}
				else{
					index = i;
				}
				(*rgba8888_buffer)[index] = extracted;
			}
		}
		else if(bpp == 8){	//PAL8BPP
			uint8_t extracted;
			for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
				if(fread(&extracted, read_size, 1, texture_file) != 1){LOAD_ERROR(30);}
				if(twiddled){
					index = get_twiddled_index(dtex_header.width, dtex_header.height, i);	//Given i, it says which element should be there to twiddle it
				}
				else{
					index = i;
				}
				(*rgba8888_buffer)[index] = extracted;
			}
		}
		else{	//PAL4BPP
			uint8_t extracted;
			for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
				if(fread(&extracted, read_size, 1, texture_file) != 1){LOAD_ERROR(30);}
				uint8_t byte_section[2];	//Extracted must be split into two vars containing the top and bottom 4 bits
				byte_section[0] = bit_extracted(extracted, 4, 4);
				byte_section[1] = bit_extracted(extracted, 4, 0);
				for(int j = 0; j < 2; j++){
					if(twiddled){
						index = get_twiddled_index(dtex_header.width, dtex_header.height, i + (1 - j));
					}
					else{
						index = i + (1 - j);
					}

					(*rgba8888_buffer)[index] = byte_section[j];
					i += j;
				}
			}
		}

		#undef LOAD_ERROR

		load_cleanup:

		fclose(texture_file);
		if(load_error){return load_error;}
	}

	//Convert from whatever format to RGBA8888
	if(rgb){
		dtex_argb_to_rgba8888(&dtex_header, *rgba8888_buffer, bpc[0], bpc[1], bpc[2], bpc[3]);
	}
	else if(paletted){
		uint8_t error = apply_palette(&dtex_header, *rgba8888_buffer, texture_path);
		if(error){return 40 + error;}	//Error goes up to 7
	}
	else if(yuv){
		yuv422_to_rgba8888(&dtex_header, *rgba8888_buffer);
	}
	else if(bumpmap){
		// Bumpmap. 16bpp, S value: 8 bits; R value: 8 bits
		// The 8-bit parameters that are specified, S and R, set the two angles
		// that define the vector to a point on a hemisphere, as shown in the
		// illustration below (Page 139. https://segaretro.org/images/7/78/DreamcastDevBoxSystemArchitecture.pdf)
	}
	else{
		printf("Somehow an unsupported texture format got through\n");
		return 48;
	}

	//Set the dimensions
	*width = dtex_header.width;
	*height = dtex_header.height;

	return 0;
}

void invalid_input(){
	printf("\nWrong number of arguments provided. This is the format\n");
	printf("./DtexToRGBA8888 [dtex_filename] *[--binary] [rgba8888_binary_filename] *[--png] [png_filename]\n");
	printf("png and binary is an optional tag that toggle png and/or binary outputs\n");
	printf("However you must choose at least one\n");

	printf("Please not the following stuff isn't supported:\n");
	printf("\t-BUMPMAP pixel format\n");
	printf("\t-Compressed\n");
	printf("\t-Mipmapped\n");

	return;
}

//Add argb8888/rgba8888 toggle.
	//rgba8888 is the default
int main(int argC, char *argV[]){	//argC is params + prog name count. So in "./prog lol 4" argC = 3 ("4" is param index 2)
	bool flag_binary_preview = false;
	uint8_t binary_index = 0;
	bool flag_png_preview = false;
	uint8_t png_index = 0;
	for(int i = 1; i < argC; i++){
		//1st param is reserved for the dtex name
		if(i == 1 && !(strlen(argV[i]) >= 4 && strcmp(argV[i] + strlen(argV[i]) - 5, ".dtex") == 0)){
			invalid_input();
			return 1;
		}

		if(!strcmp(argV[i], "--binary")){
			if(i + 1 >= argC){
				invalid_input();
				return 1;
			}
			flag_binary_preview = true;
			binary_index = i + 1;
		}

		if(!strcmp(argV[i], "--png")){
			if(i + 1 < argC && strlen(argV[i + 1]) >= 4 && strcmp(argV[i + 1] + strlen(argV[i + 1]) - 4, ".png") == 0){
				flag_png_preview = true;
				png_index = i + 1;
			}
			else{
				invalid_input();
				return 1;
			}
		}
	}

	if(!flag_binary_preview && !flag_png_preview){
		invalid_input();
		return 1;
	}

	uint32_t * texture = NULL;
	uint16_t height = 0;
	uint16_t width = 0;

	uint8_t error_code = load_dtex(argV[1], &texture, &height, &width);
	if(error_code){
		printf("Error %d has occurred. Terminating program\n", error_code);
		free(texture);
		return 1;
	}

	if(flag_binary_preview){
		FILE * f_binary = fopen(argV[binary_index], "wb");
		if(f_binary == NULL){
			printf("Error opening file!\n");
			return 1;
		}
		fwrite(texture, sizeof(uint32_t), height * width, f_binary);	//Note this is stored in little endian
		fclose(f_binary);
	}

	//Output a PNG if requested
	if(flag_png_preview){
		png_details_t p_det;
		if(rgba8888_to_png_details(texture, height, width, &p_det)){return 1;}
		write_png_file(argV[png_index], &p_det);
	}
	free(texture);

	return 0;
}
