#include "bitmap.h"

uint32_t bitmap_flip_byte(unsigned char* _v, int _c)
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

struct bitmap_pixel_data bitmap_read(char *_file)
{
  struct bitmap_pixel_data ret;
  struct bitmap_file_header header;

  ret.R = ret.G = ret.B = NULL;

  FILE *bitmap = fopen(_file,"r");

  if(!bitmap)
    return ret;

  header = bitmap_read_file_header(bitmap);

  if(header.error)
    return ret;

  if(header.biBitCount != 24)
    return ret;

  if(header.biCompression != 0)
    return ret;

  ret = bitmap_read_pixel_data(bitmap, header);

  fclose(bitmap);

  return ret;
}

struct bitmap_file_header bitmap_read_file_header(FILE *_file)
{
  struct bitmap_file_header ret;
  unsigned char fileheader[_HEADER_SIZE];
  uint32_t      read_counter = 0;

  ret.error = 1;

  size_t tt = fread((void*)&fileheader, sizeof(char), _HEADER_SIZE, _file);
  read_counter += _HEADER_SIZE;

  if(!tt)
    return ret;

  //Copy file header
  ret.bfType =          (uint16_t) bitmap_flip_byte(&fileheader[BF_TYPE], sizeof(ret.bfType));

  if(ret.bfType != (uint16_t)IDENTIFIER)
    return ret;

  ret.bfSize =          (uint32_t) bitmap_flip_byte(&fileheader[BF_SIZE], sizeof(ret.bfSize));
  ret.bfOffBits =      *(uint32_t*) &fileheader[BF_OFF_BITS];
  ret.biSize =         *(uint32_t*) &fileheader[BI_SIZE];
  ret.biWidth =        *(int32_t*) &fileheader[BI_WIDTH];
  ret.biHeight =       *(int32_t*) &fileheader[BI_HEIGHT];
  ret.biBitCount =     *(uint16_t*) &fileheader[BI_BIT_COUNT];
  ret.biCompression =   (uint32_t) bitmap_flip_byte(&fileheader[BI_COMPRESSION], sizeof(ret.biCompression));
  ret.biSizeImage =    *(uint32_t*) &fileheader[BI_SIZE_IMAGE];
  ret.biClrUsed =       (uint32_t) bitmap_flip_byte(&fileheader[BI_CLR_USED], sizeof(ret.biClrUsed));
  ret.biClrImportant =  (uint32_t) bitmap_flip_byte(&fileheader[BI_CLR_IMPORTANT], sizeof(ret.biClrImportant));


  //Read to start of Pixel block
  //This block contains Colormasks and Colortables.
  ret.tablesc = ret.bfOffBits - read_counter;
  ret.tables = malloc(sizeof(char)* ret.tablesc);
  fread(ret.tables, sizeof(char), ret.tablesc, _file);
  //////////

  ret.error = 0;
  return ret;
}

struct bitmap_pixel_data bitmap_read_pixel_data(FILE *_file, struct bitmap_file_header _header)
{
  uint32_t      **bitmap_buff;

  struct bitmap_pixel_data ret;

  uint32_t row_size = _header.biWidth * 3;
  while(row_size%4)
    row_size++;

  ret.x = _header.biWidth;
  ret.y = _header.biHeight < 0 ? -_header.biHeight: _header.biHeight;

  //If biHeight > 0 Data starts with last row!!

  //Allocate 2D array
  //!!
  //bitmap_buff indeces are flipped!! [y][x]!!!!!
  bitmap_buff = malloc(sizeof(*bitmap_buff) * _header.biHeight);
  for(int i = 0; i < ret.y; i++)
  {
    bitmap_buff[i] = malloc(sizeof(*bitmap_buff[i]) * _header.biWidth);
  }

  //Copy Bitmap into bitmap_buff
  for(int row = 0; row < _header.biHeight; row++)
  {
    //printf("Row %i\n", row);
    //fread(bitmap_buff[row], sizeof(char), row_size, bitmap);
    for(int col = 0; col < _header.biWidth; col++)
      fread(&bitmap_buff[row][col], 1, 3, _file);

    for(int i = 0; i < row_size - (_header.biWidth * 3); i++) //read excess NULL-Bytes
        fgetc(_file);
  }

  ret.x = _header.biWidth;
  ret.y = _header.biHeight < 0 ? -_header.biHeight: _header.biHeight;

  ret.R = malloc(sizeof(*ret.R) * ret.x);
  ret.G = malloc(sizeof(*ret.G) * ret.x);
  ret.B = malloc(sizeof(*ret.B) * ret.x);
  for(int i = 0; i < ret.y; i++)
  {
    ret.R[i] = malloc(sizeof(*ret.R[i]) * ret.y);
    ret.G[i] = malloc(sizeof(*ret.G[i]) * ret.y);
    ret.B[i] = malloc(sizeof(*ret.B[i]) * ret.y);
  }

  for(int y = 0; y < ret.y; y++)
  {
    for(int x = 0; x < ret.x; x++)
    {
      int row = _header.biHeight > 0 ? (ret.y - 1) - y : y;

      ret.R[x][y] = (bitmap_buff[row][x] & 0xff0000)>>16;
      ret.G[x][y] = (bitmap_buff[row][x] & 0x00ff00)>>8;
      ret.B[x][y] = (bitmap_buff[row][x] & 0x0000ff);
    }
  }

  for(int i = 0; i < ret.y; i++)
    free(bitmap_buff[i]);
  free(bitmap_buff);

  return ret;
}
