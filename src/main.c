#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "bitmap.h"
#include "character.h"
#include "m.h"
#include "color.h"

/* #define CLEANUP 1 */

#ifdef _DEBUG
#warning "Compiling with DEBUG"
#define DEBUG_PRINTF(...) { printf(__VA_ARGS__); }
#else
#define DEBUG_PRINTF(...) {	}
#endif

#define CHAR_SIZE_X 2 //How many pixels should form one ASCII char?
#define CHAR_SIZE_Y (2 * CHAR_SIZE_X)

struct prog_param
{
	char *filename;
	char *character_map;
	unsigned int charsize_x;
	unsigned int charsize_y;
	uint8_t color;
	uint8_t use_stdin;
	uint8_t use_whitespace;
	uint8_t fit_width;
	uint8_t dynamic_range;
};

struct prog_param parse_args(int argc, char *argv[]); 

void print_help( void );

int main(int argc, char *argv[])
{
	struct prog_param args = parse_args(argc, argv);

	//Stores a luminance array
	uint8_t	**image_monochrome;
	//Stores a color array
	char*	**col_buff;

	uint8_t brightness_max = 0x00;
	uint8_t brightness_min = 0xff;

	struct bitmap_pixel_data bitmap;
	bitmap = bitmap_read(args.filename);

	if(bitmap.error) {
		printf("Error reading file\n");
		return 1;
	}

	// Character count in x and y in final ASCII image
	unsigned int size_x,size_y;

	if(args.fit_width > 0) {
		args.charsize_x = (unsigned int)((float)bitmap.x / (float)args.fit_width);
		args.charsize_y = (unsigned int)(((float)bitmap.y / (float)bitmap.x) * (float)args.charsize_x * 2);
	}
	size_x = bitmap.x / args.charsize_x;
	size_y = bitmap.y / args.charsize_y;

	DEBUG_PRINTF("Output size: %u x %u\n", size_x, size_y);

	//Allocate character sotrage
	image_monochrome = malloc(sizeof(*image_monochrome) * size_x);
	for (int i = 0; i < size_x; i++) 
		image_monochrome[i] = malloc(sizeof(image_monochrome[i]) * size_y);

	//Allocate color storage if color enabled
	if(args.color) {
		col_buff = malloc(sizeof(*col_buff) * size_x);
		for (int i = 0; i < size_x; i++)
			col_buff[i] = malloc(sizeof(col_buff[i]) * size_y);
	}

	//Nest thine Lööps
	//Very not optimal Variable names!!!!!!!!!!!!!
	//
	//For every size_x * size_y block: calculate average values of pixel blocks
	for(unsigned int x = 0; x < size_x; x++) {
		for(unsigned int y = 0; y < size_y; y++) {
			/* Luminance for every pixel */
			uint8_t brightness [ args.charsize_x ][ args.charsize_y ];
			/* Color for every Pixel */
			uint8_t color_list[ 3 ][ args.charsize_x * args.charsize_y ]; //RGB Values of Pixels, used for averaging
			unsigned int color_list_counter = 0;

			/* Iterate through pixel block, save brightness and color if set */
			for(unsigned int row_c = 0; row_c < args.charsize_y; row_c++) {
				unsigned int row = y * args.charsize_y + row_c; //Actual position in Bitmap

				for(unsigned int col_c = 0; col_c < args.charsize_x; col_c++) {
					unsigned int col = x * args.charsize_x + col_c; //Actual position in bitmap

					brightness[col_c][row_c] = rgb_luminance(
							bitmap.R[col][row],
							bitmap.G[col][row],
							bitmap.B[col][row]);

					if(args.color) {
						color_list[0][color_list_counter] = bitmap.R[col][row];
						color_list[1][color_list_counter] = bitmap.G[col][row];
						color_list[2][color_list_counter] = bitmap.B[col][row];
						color_list_counter++;
					}//if
				}//for col_c
			}//for row_c

			/* Calculate average brightness in pixel block */
			image_monochrome[x][y] = avg(args.charsize_x * args.charsize_y, *brightness);

			/* Calculate average color in pixel block */
			if(args.color) {
				if(args.use_whitespace)
					col_buff[x][y] = calc_bg_col_ansi(
						(uint8_t)avg(args.charsize_x * args.charsize_y, color_list[0]),
						(uint8_t)avg(args.charsize_x * args.charsize_y, color_list[1]),
						(uint8_t)avg(args.charsize_x * args.charsize_y, color_list[2]));
				else
					col_buff[x][y] = calc_col_ansi(
						(uint8_t)avg(args.charsize_x * args.charsize_y, color_list[0]),
						(uint8_t)avg(args.charsize_x * args.charsize_y, color_list[1]),
						(uint8_t)avg(args.charsize_x * args.charsize_y, color_list[2]));
			} // if args.color

			/* Save min and max brightness values for dynamic range */
			if((uint8_t)image_monochrome[x][y] < brightness_min)
				brightness_min = image_monochrome[x][y];
			if((uint8_t)image_monochrome[x][y] > brightness_max)
				brightness_max = image_monochrome[x][y];
		}//for y
	}//for x


	/* Apply Default Colors */
	if(args.color)
		printf("\e[0m");
	
	if(! args.dynamic_range) {
		brightness_min = 0;
		brightness_max = 255;
	} else {
		DEBUG_PRINTF("Dynamic Range: Brightness Values: Min: %u Max: %u\n", brightness_min, brightness_max);
	}

	/* Print the buffer */
	for(int y = 0; y<size_y; y++) {
		for(int x = 0; x < size_x; x++) {
			if(args.color) 
				printf("\e[%sm", col_buff[x][y]);
			if(args.use_whitespace)
				printf(" ");
			else
				printf("%c", calc_char(image_monochrome[x][y], brightness_min, brightness_max, args.character_map));
		}
		printf("\e[0m\n");
	}

	/* Apply Default Colors */
	if(args.color)
		printf("\e[0m");

	DEBUG_PRINTF("Finished!\n");

#ifdef CLEANUP
	for(int i = 0; i < size_x; i++)
		free (image_monochrome[i]);
	free(image_monochrome);

	for(int i = 0; i < bitmap.x; i++) {
		free(bitmap.R[i]);
		free(bitmap.G[i]);
		free(bitmap.B[i]);
	}
	free(bitmap.R);
	free(bitmap.G);
	free(bitmap.B);
	if(args.color) {
		for(int i = 0; i < size_x; i++)
			free(col_buff[i]);
		free(col_buff);
	}
#endif

	return 0;
}//main

struct prog_param parse_args(int argc, char *argv[])
{
	struct prog_param ret;

	memset(&ret, 0, sizeof ret);

	ret.charsize_x = CHAR_SIZE_X;

	for (int i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			int icpy = i;
			for(int o = 1; o < strlen(argv[icpy]); o++) {
				switch(argv[icpy][o]) {
					case 'h':
						print_help();
						exit(0);
						break;
					case 'x':
						ret.charsize_x = atoi(argv[++i]);
						break;
					case 'y':
						ret.charsize_y = atoi(argv[++i]);
						break;
					case 'c':
						ret.color = 1;
						break;
					case 'i':
						ret.use_stdin = 1;
						break;
					case 'w':
						ret.use_whitespace = 1;
						break;
					case 's':
						ret.fit_width = atoi(argv[++i]);
						break;
					case 'd':
						ret.dynamic_range = 1;
						break;
					case 'm':
						ret.character_map = argv[++i];
						break;
					default:
						printf("Unrecognized Option '%c'\n", argv[icpy][o]);
						print_help();
						exit(1);
				};//switch
			}//for o
		}//if
		else if(ret.filename == NULL && !ret.use_stdin ) {
			ret.filename = argv[i];
		} else {
			printf("Wrong number of arguments\n");
			print_help();
			exit(1);
		}
	}//for i

	if(ret.filename == NULL && !ret.use_stdin )
	{
		printf("No input file. Use -i to read from stdin\n");
		print_help();
		exit(1);
	}

	if(!ret.charsize_y)
		ret.charsize_y = 2 * ret.charsize_x;

	return ret;
}

void print_help( void )
{
	printf("ASCIIMap\n(c) 2019 Jonas Gunz, github.com/kompetenzbolzen/AsciiMap\n");
	printf("ASCIIMap prints a ASCII representation of a bitmap image\n\nUsage: [OPTIONS] FILENAME\n");
	printf("Options:\n	-h: Print this help message\n	-x VAL: set the width of block wich makes up one character. Default: %i\n", CHAR_SIZE_X);
	printf("	-y VAL: set the height of block wich makes up one character. Default: 2*x\n	-c: Print in ANSI color mode. Default: OFF\n");
	printf("	-i: Read from STDIN instead of file.\n	-w: print only whitespaces with background color\n");
	printf("	-d: Dynamic brightness range\n	-m PALETTE: specify custom character palette from darkest to brightest\n");
}


