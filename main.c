#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

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
  char          **ascii_buff;

  uint8_t b_max = 0x00;
  uint8_t b_min = 0xff;

  struct bitmap_pixel_data bitmap;

  if(argc != 3)
  {
    printf("Usage: %s <input> <output>\n", argv[0]);
    return 1;
  }

  bitmap = bitmap_read(argv[1]);

  if(bitmap.error)
  {
    printf("Error reading file\n");
    return 1;
  }

  //Calculate Averages of CHAR_SIZE x CHAR_SIZE blocks
  unsigned int size_x,size_y;
  size_x = bitmap.x  / CHAR_SIZE_X;
  size_y = bitmap.y / CHAR_SIZE_Y;

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
      char b[CHAR_SIZE_X][CHAR_SIZE_Y];

      for(int r = 0; r < CHAR_SIZE_Y; r++)
      {
        int row = y * CHAR_SIZE_Y + r;
        for(int c = 0; c < CHAR_SIZE_X; c++)
        {
          int col = x * CHAR_SIZE_X + c;
          //b[c][r] = avg(3, (char*)&bitmap_buff[row][col]);
          b[c][r] = rgb_avg(bitmap.R[col][row],bitmap.G[col][row],bitmap.B[col][row]);
        }
      }

      ascii_buff[x][y] = avg(CHAR_SIZE_X * CHAR_SIZE_Y, (char*)&b);

      if((uint8_t)ascii_buff[x][y] < b_min)
        b_min = ascii_buff[x][y];
      if((uint8_t)ascii_buff[x][y] > b_max)
        b_max = ascii_buff[x][y];
    }
  }

  DEBUG_PRINTF("Brightness Values: Min: %u Max: %u\n", b_min, b_max);

  //Write Output
  DEBUG_PRINTF("Opening %s for writing.\n", argv[2]);
  FILE *out = fopen(argv[2], "w");

  if(!out)
  {
    printf("Error opening output File. Check writing permissions.\n");
    return 1;
  }

  for(int y = 0; y<size_y; y++)
  {
    for(int x = 0; x < size_x; x++)
    {
      fputc( calc_char(ascii_buff[x][y], b_min, b_max) , out);
    }
    DEBUG_PRINTF(".");
    fputc('\n', out);
  }
  DEBUG_PRINTF("\n");

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

  fclose(out);

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
