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

#define CHAR_SIZE_X 2 //How many pixels should form one ASCII char?
#define CHAR_SIZE_Y (2 * CHAR_SIZE_X)

const char map[] = {' ', ' ', '.', ',', '`', '-', '~', '"', '*',  ':', ';', '<', '!', '/','?', '%', '&', '=', '$', '#'};
//const char map[] = {'`', '.', ',', ':', ';', '\'', '+', '#', '@'};
//Routine for flipping bytes
uint32_t flip(unsigned char* _v, int _c);

//Calculate average
char avg(int argc, char *argv);

//Calculate luminance from rgb_avg
//Order LSB first: BGR
char rgb_avg(uint32_t *arg);

//Select Char based on 1B brightness Value
char calc_char(uint8_t _c, uint8_t _min, uint8_t _max);

int main(int argc, char *argv[])
{
  unsigned char fileheader[_HEADER_SIZE];
  unsigned char *tables;
  uint32_t      **bitmap_buff;
  char          **ascii_buff;
  uint32_t      read_counter = 0;

  uint8_t b_max = 0x00;
  uint8_t b_min = 0xff;

  uint16_t bfType         = 0;
  uint32_t bfSize         = 0;
  uint32_t bfOffBits      = 0;

  uint32_t biSize         = 0;
  int32_t  biWidth        = 0;
  int32_t  biHeight       = 0;
  uint16_t biBitCount     = 0;
  uint32_t biCompression  = 0;
  uint32_t biSizeImage    = 0;
  uint32_t biClrUsed      = 0;
  uint32_t biClrImportant = 0;

  if(argc != 3)
  {
    printf("Usage: %s <input> <output>\n", argv[0]);
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
  uint32_t haeder_end = bfOffBits - read_counter;
  tables = malloc(sizeof(char)* haeder_end);
  fread(tables, sizeof(char), haeder_end, bitmap);
  read_counter += haeder_end;
  printf("Data starts at %x\n", read_counter);

  //One pixel is 3Byte, One line is multiple of 4Bytes
  uint32_t row_size = biWidth * 3;
  while(row_size%4)
    row_size++;

  printf("Set row reading size to %uB\n", row_size);
  //If biHeight > 0 Data starts with last row!!

  //Allocate 2D array
  //!!
  //bitmap_buff indeces are flipped!! [y][x]!!!!!
  bitmap_buff = malloc(sizeof(*bitmap_buff) * biHeight);
  for(int i = 0; i < biHeight; i++)
  {
    bitmap_buff[i] = malloc(sizeof(*bitmap_buff[i]) * biWidth);
  }

  //Copy Bitmap into bitmap_buff
  for(int row = 0; row < biHeight; row++)
  {
    //printf("Row %i\n", row);
    //fread(bitmap_buff[row], sizeof(char), row_size, bitmap);
    for(int col = 0; col < biWidth; col++)
      fread(&bitmap_buff[row][col], 1, 3, bitmap);

    read_counter += row_size;
  }

  printf("Finished copying Bitmap\n");

  //Calculate Averages of CHAR_SIZE x CHAR_SIZE blocks
  unsigned int size_x,size_y;
  size_x = biWidth  / CHAR_SIZE_X;
  size_y = biHeight / CHAR_SIZE_Y;

  printf("Creating ASCII File %u x %u\n", size_x, size_y);

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
          b[c][r] = rgb_avg(&bitmap_buff[row][col]);
        }
      }

      ascii_buff[x][y] = avg(CHAR_SIZE_X * CHAR_SIZE_Y, (char*)&b);

      if((uint8_t)ascii_buff[x][y] < b_min)
        b_min = ascii_buff[x][y];
      if((uint8_t)ascii_buff[x][y] > b_max)
        b_max = ascii_buff[x][y];
    }
  }

  printf("Brightness Values: Min: %u Max: %u\n", b_min, b_max);

  //Write Output
  printf("Opening %s for writing.\n", argv[2]);
  FILE *out = fopen(argv[2], "w");

  if(!out)
  {
    printf("Error opening output File. Check writing permissions.\n");
    return 1;
  }

  for(int y = size_y; y >= 0; y--)
  {
    for(int x = 0; x < size_x; x++)
    {
      fputc( calc_char(ascii_buff[x][y], b_min, b_max) , out);
    }
    printf(".");
    fputc('\n', out);
  }
  printf("\n");

  printf("Finished!\n");

  //Cleanup
  for(int i = 0; i < biHeight; i++)
    free (bitmap_buff[i]);
  free(bitmap_buff);

  for(int i = 0; i < size_y; i++)
    free (ascii_buff[i]);
  free(ascii_buff);

  fclose(bitmap);
  fclose(out);

  return 0;
}
//One pixel is 3Byte, One line is multiple of 4Bytes
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
}//avg

char calc_char(uint8_t _c , uint8_t _min, uint8_t _max)
{
  float c = (float)(_c) / (_max - _min);
  return map [(int)((sizeof(map)-1) * (1-c)) ];
}

char rgb_avg(uint32_t *arg)
{
  char ret;
  uint32_t R = (*arg & 0xff0000)>>16;;
  uint32_t G = (*arg & 0xff00)>>8;
  uint32_t B = *arg & 0xff;
  ret = (char)(R+R+B+G+G+G)/6;
  return ret;
}
