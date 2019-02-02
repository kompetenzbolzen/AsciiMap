#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


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

#define CHAR_SIZE 10 //How many pixels should form one ASCII char?

const char map[] = {' ', '.', ',', '-', ':', ';', '!', '/','?' , '%', '$', '#'};

struct ascii_pixel
{
  char i[3 * CHAR_SIZE][CHAR_SIZE]; //
  char c;
};

//Routine for flipping bytes
uint32_t flip(unsigned char* _v, int _c);

//Calculate average
char avg(int argc, char *argv);

int main(int argc, char *argv[])
{
  unsigned char fileheader[_HEADER_SIZE];
  unsigned char *tables;
  uint32_t **bitmap_buff;
  uint32_t read_counter = 0;

  uint16_t bfType = 0;
  uint32_t bfSize = 0;
  uint32_t bfOffBits = 0;

  uint32_t biSize = 0;
  int32_t  biWidth = 0;
  int32_t  biHeight = 0;
  uint16_t biBitCount = 0;
  uint32_t biCompression = 0;
  uint32_t biSizeImage = 0;
  uint32_t biClrUsed = 0;
  uint32_t biClrImportant = 0;

  if(argc < 2)
  {
    printf("No input file specified. Abort.\n");
    return 1;
  }

  printf("Opening %s\n", argv[1]);

  FILE *bitmap;
  bitmap = fopen(argv[1], "r");

  if(bitmap==NULL)
  {
    printf("Error opening file. Abort.\n");
    return 1;
  }

  size_t tt = fread((void*)&fileheader, sizeof(char), _HEADER_SIZE, bitmap);
  read_counter += _HEADER_SIZE;

  if(!tt)
  {
    printf("Error reading file. Abort.\n");
    return 1;
  }

  //Copy file header
  bfType =          (uint16_t) flip(&fileheader[BF_TYPE], sizeof(bfType));

  if(bfType != (uint16_t)IDENTIFIER)
  {
    printf("%s is not a valid Bitmap. Abort.\n", argv[1]);
    fclose(bitmap);
    return 1;
  }

  bfSize =          (uint32_t) flip(&fileheader[BF_SIZE], sizeof(bfSize));
  bfOffBits =      *(uint32_t*) &fileheader[BF_OFF_BITS];
  biSize =         *(uint32_t*) &fileheader[BI_SIZE];
  biWidth =        *(int32_t*) &fileheader[BI_WIDTH];
  biHeight =       *(int32_t*) &fileheader[BI_HEIGHT];
  biBitCount =     *(uint16_t*) &fileheader[BI_BIT_COUNT];
  biCompression =   (uint32_t) flip(&fileheader[BI_COMPRESSION], sizeof(biCompression));
  biSizeImage =    *(uint32_t*) &fileheader[BI_SIZE_IMAGE];
  biClrUsed =       (uint32_t) flip(&fileheader[BI_CLR_USED], sizeof(biClrUsed));
  biClrImportant =  (uint32_t) flip(&fileheader[BI_CLR_IMPORTANT], sizeof(biClrImportant));

  printf("Picture is %u x %u Pixels. Colordepth: %ubit. Size: %uB\n",biWidth, biHeight, biBitCount, biSizeImage);

  if(biBitCount !=24)
  {
    printf("%ubit mode not supported.\n", biBitCount);
    return 1;
  }
  if(biCompression != 0)
  {
    printf("Compression not supported.\n");
    return 1;
  }

  //Read to start of Pixel block
  //This block contains Colormasks and Colortables.
  //Unused
  uint32_t d = bfOffBits - read_counter;
  tables = malloc(sizeof(char)* d);
  fread(tables, sizeof(char), d, bitmap);

  read_counter += d;
  printf("Read to %x\n", read_counter);

  uint32_t row_size = biWidth * 3; //One pixel is 3Byte, One line is multiple of 4Bytes
  while(row_size%4)
    row_size++;

  printf("Set row reading size to %uB\n", row_size);
  //If biHeight > 0 Data starts with last row!!

  //Allocate 2D array
  bitmap_buff = malloc(sizeof(*bitmap_buff) * biWidth);
  for(int i = 0; i < biWidth; i++)
  {
    bitmap_buff[i] = malloc(sizeof(*bitmap_buff[i]) * biHeight);
  }


  for(int i = 0; i < biWidth; i++)
    free(bitmap_buff[i]);
  free(bitmap_buff);

  fclose(bitmap);
  return 0;
}

uint32_t flip(unsigned char* _v, int _c)
{
  uint32_t ret = 0;
  uint32_t counter = (_c-1) * 8;

  for(int i = 0; i < _c; i++)
  {
    ret |= (uint32_t)(_v[i] << (counter));
    counter -= 8;
  }

  return ret;
}//flip

char avg(int argc, char *argv)
{
  char ret = 0;
  uint64_t sum = 0;

  for(int i = 0; i < argc; i++)
    sum += (uint64_t)argv[i];

  ret = (char)(sum / argc);

  return ret;
}//flip
