#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

#define R(x) (0xff0000 & x) >> 16
#define G(x) (0x00ff00 & x) >> 8
#define B(x) (0x0000ff & x)

struct bitmap_file_header
{
  uint8_t error;

  uint16_t bfType;
  uint32_t bfSize;
  uint32_t bfOffBits;

  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  uint32_t biClrUsed;
  uint32_t biClrImportant;

  unsigned char *tables;
  uint32_t      tablesc;
};

struct bitmap_pixel_data
{
  unsigned int x,y;
  uint8_t **R;
  uint8_t **G;
  uint8_t **B;
};


uint32_t bitmap_flip_byte(unsigned char* _v, int _c);

struct bitmap_pixel_data bitmap_read(char *_file);

struct bitmap_file_header bitmap_read_file_header(FILE *_file);

struct bitmap_pixel_data bitmap_read_pixel_data(FILE *_file, struct bitmap_file_header _header);


#endif /* end of include guard: _BITMAP_H_ */
