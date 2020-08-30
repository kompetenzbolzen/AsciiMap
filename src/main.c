/*
 * src/main.c
 * (c) 2020 Jonas Gunz <himself@jonasgunz.de>
 * License: MIT
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "bitmap.h"
#include "character.h"

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

	struct bitmap_image bitmap;
	struct bitmap_image shrunk_bitmap;
	struct bitmap_image monochrome_bitmap;

	uint8_t brightness_min = 0x00;
	uint8_t brightness_max = 0xff;

	if ( bitmap_read(args.filename, &bitmap) ) {
		printf("Error reading file\n");
		return 1;
	}

	if(args.fit_width > 0) {
		args.charsize_x = (unsigned int)((float)bitmap.x / (float)args.fit_width);
		args.charsize_y = (unsigned int)(((float)bitmap.y / (float)bitmap.x) * (float)args.charsize_x * 2);
	}

	bitmap_shrink ( &bitmap, &shrunk_bitmap, args.charsize_x, args.charsize_y );
	bitmap_convert_monochrome ( &shrunk_bitmap, &monochrome_bitmap );

	if( args.dynamic_range ) {
		brightness_min = monochrome_bitmap.monochrome_minimum_brightness;
		brightness_max = monochrome_bitmap.monochrome_maximum_brightness;
		DEBUG_PRINTF("Dynamic Range: Brightness Values: Min: %u Max: %u\n", brightness_min, brightness_max);
	}

	/* Apply Default Colors */
	if(args.color)
		printf("\e[0m");

	/* Print the buffer */
	uint8_t color_mode = args.use_whitespace ? COLOR_BG : COLOR_FG;
	for(int y = 0; y < monochrome_bitmap.y; y++) {
		for(int x = 0; x < monochrome_bitmap.x; x++) {
			char c = args.use_whitespace ? ' ' : calc_char(monochrome_bitmap.R[x][y], brightness_min, brightness_max, args.character_map);

			if(args.color) 
				printf("%s", calc_col_ansi(
							shrunk_bitmap.R[x][y],
							shrunk_bitmap.G[x][y],
							shrunk_bitmap.B[x][y],
							color_mode ) );
			printf("%c", c);
		}
		printf("\e[0m\n");
	}

	/* Apply Default Colors */
	if(args.color)
		printf("\e[0m");

	DEBUG_PRINTF("Finished!\n");

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

	if(ret.use_whitespace && !ret.color) {
		printf("use -w only with -c\n");
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

