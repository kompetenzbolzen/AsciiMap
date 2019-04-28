#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "bitmap.h"

#ifdef _DEBUG
#warning "Compiling with DEBUG"
#define DEBUG_PRINTF(...) { printf(__VA_ARGS__); }
#else
#define DEBUG_PRINTF(...) {  }
#endif

#define _HEADER_SIZE 54 //Fileheader + infoheader
#define IDENTIFIER 0x424d //BM BitMap identifier

//Address Definitions
#define BF_TYPE           0x00
#define BF_SIZE           0x02
#define BF_OFF_BITS       0x0a

#define BI_SIZE           0x0e
#define BI_WIDTH          0x12
#define BI_HEIGHT         0x16
#define BI_BIT_COUNT      0x1c
#define BI_COMPRESSION    0x1e
#define BI_SIZE_IMAGE     0x22
#define BI_CLR_USED       0x2e
#define BI_CLR_IMPORTANT  0x32

#define CHAR_SIZE_X 2 //How many pixels should form one ASCII char?
#define CHAR_SIZE_Y (2 * CHAR_SIZE_X)

struct prog_param
{
	char *filename;
	unsigned int charsize_x;
	unsigned int charsize_y;
};

struct prog_param parse_args(int argc, char *argv[]);

void print_help( void );

const char map[] = {' ', ' ', '.', ',', '`', '-', '~', '"', '*', ':', ';', '<', '!', '/', '?', '%', '&', '=', '$', '#'};
//const char map[] = {' ', '`', '.', ',', ':', ';', '\"', '+', '#', '@'};

//Calculate average
char avg(int argc, char *argv);

//Calculate luminance from rgb_avg
//Order LSB first: BGR
char rgb_avg(uint8_t R, uint8_t G, uint8_t B);

//Select Char based on 1B brightness Value
char calc_char(uint8_t _c, uint8_t _min, uint8_t _max);

int main(int argc, char *argv[])
{
  struct prog_param args = parse_args(argc, argv);

  char          **ascii_buff;

  uint8_t b_max = 0x00;
  uint8_t b_min = 0xff;

  struct bitmap_pixel_data bitmap;


  bitmap = bitmap_read(args.filename);

  if(bitmap.error)
  {
    printf("Error reading file\n");
    return 1;
  }

  //Calculate Averages of CHAR_SIZE x CHAR_SIZE blocks
  unsigned int size_x,size_y;
  size_x = bitmap.x  / args.charsize_x;
  size_y = bitmap.y / args.charsize_y;

  DEBUG_PRINTF("Creating ASCII File %u x %u\n", size_x, size_y);

  ascii_buff = malloc(sizeof(*ascii_buff) * size_x);
  for (int i = 0; i < size_x; i++)
  {
    ascii_buff[i] = malloc(sizeof(ascii_buff[i]) * size_y);
  }

  //Nest thine Lööps
  for(int x = 0; x < size_x; x++)
  {
    for(int y = 0; y < size_y; y++)
    {
      char b[args.charsize_x][args.charsize_y];

      for(int r = 0; r < args.charsize_y; r++)
      {
        int row = y * args.charsize_y + r;
        for(int c = 0; c < args.charsize_x; c++)
        {
          int col = x * args.charsize_x + c;
          //b[c][r] = avg(3, (char*)&bitmap_buff[row][col]);
          b[c][r] = rgb_avg(bitmap.R[col][row],bitmap.G[col][row],bitmap.B[col][row]);
        }
      }

      ascii_buff[x][y] = avg(args.charsize_x * args.charsize_y, (char*)&b);

      if((uint8_t)ascii_buff[x][y] < b_min)
        b_min = ascii_buff[x][y];
      if((uint8_t)ascii_buff[x][y] > b_max)
        b_max = ascii_buff[x][y];
    }
  }

  DEBUG_PRINTF("Brightness Values: Min: %u Max: %u\n", b_min, b_max);


  for(int y = 0; y<size_y; y++)
  {
    for(int x = 0; x < size_x; x++)
    {
      printf("%c", calc_char(ascii_buff[x][y], b_min, b_max));
    }
    printf("\n");
  }

  DEBUG_PRINTF("Finished!\n");

  //Cleanup
  for(int i = 0; i < size_x; i++)
    free (ascii_buff[i]);
  free(ascii_buff);

  for(int i = 0; i < bitmap.x; i++)
  {
    free(bitmap.R[i]);
    free(bitmap.G[i]);
    free(bitmap.B[i]);
  }
  free(bitmap.R);
  free(bitmap.G);
  free(bitmap.B);

  return 0;
}//main


char avg(int argc, char *argv)
{
  char ret = 0;
  uint64_t sum = 0;

  for(int i = 0; i < argc; i++)
    sum += (uint64_t)argv[i];

  ret = (char)(sum / argc);

  return ret;
}//avg

char calc_char(uint8_t _c , uint8_t _min, uint8_t _max)
{
  float c = (float)(_c) / (_max - _min);
  return map [(int)((sizeof(map)-1) * (c))];
}

char rgb_avg(uint8_t R, uint8_t G, uint8_t B)
{
  char ret;

  ret = sqrt( 0.299*pow(R,2) + 0.587*pow(G,2) + 0.114*pow(B,2) ); //(char)(R+R+B+G+G+G)/6;

  return ret;
}

struct prog_param parse_args(int argc, char *argv[])
{
	struct prog_param ret;

	ret.filename = NULL;
	ret.charsize_x = CHAR_SIZE_X;
	ret.charsize_y = 0;

	for (int i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			int icpy = i;
			for(int o = 1; o < strlen(argv[icpy]); o++)
			{
				switch(argv[icpy][o])
				{
					case 'h':
						print_help();
						exit(1);
						break;
					case 'x':
						DEBUG_PRINTF("x\n");
						i++;
						ret.charsize_x = atoi(argv[i]);
						break;
					case 'y':
						DEBUG_PRINTF("y\n");
						i++;
						ret.charsize_y = atoi(argv[i]);
						break;
					default:
						printf("Unrecognized Option\n");
						print_help();
						exit(1);

				};
			}
		}
		else if(ret.filename == NULL)
		{
			ret.filename = argv[i];
		}
		else
		{
			printf("Wrong number of arguments\n");
			print_help();
			exit(1);
		}
	}

	if(ret.filename == NULL)
	{
		printf("No input file.\n");
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
	printf("	-y VAL: set the height of block wich makes up one character. Default: 2*x\n\n");
}
