/*
 * src/bitmap.c
 * (c) 2020 Jonas Gunz <himself@jonasgunz.de>
 * License: MIT
*/

#include "bitmap.h"

static const int   bitmap_errors_cnt = 5;
static const char* bitmap_errors[] = {
	"OK.",
	"Error opening file.", // Use errno instead
	"Invalid or corrupted file.",
	"Unsupported bit depth.",
	"Compression not supported."
};

static struct bitmap_file_header bitmap_read_file_header(FILE *_file);

static struct bitmap_image bitmap_read_pixel_data(FILE *_file, struct bitmap_file_header _header);

static uint32_t bitmap_flip_byte(unsigned char* _v, int _c);

static uint8_t bitmap_rgb_luminance(uint8_t R, uint8_t G, uint8_t B);

static uint32_t bitmap_flip_byte(unsigned char* _v, int _c)
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

int bitmap_read(char *_file, struct bitmap_image *_bitmap)
{
	if ( !_bitmap )
		return 5;

	struct bitmap_file_header header;
	_bitmap->tags = 0x00;

	FILE *input_file;
	if(_file)
		input_file = fopen(_file,"rb");
	else
		input_file = stdin;

	if(!input_file)
		return 1;

	header = bitmap_read_file_header(input_file);

	if(header.error)
		return 2;

	if(header.biBitCount != 24)
		return 3;

	if(header.biCompression != 0)
		return 4;

	*_bitmap = bitmap_read_pixel_data(input_file, header);

	free(header.tables);
	fclose(input_file);

	return 0;
}

static struct bitmap_file_header bitmap_read_file_header(FILE *_file)
{
	struct bitmap_file_header ret;
	unsigned char fileheader[_HEADER_SIZE];
	uint32_t			read_counter = 0;

	ret.error = 1;

	size_t tt = fread((void*)&fileheader, sizeof(char), _HEADER_SIZE, _file);
	read_counter += _HEADER_SIZE;

	if(!tt)
		return ret;

	//Copy file header
	ret.bfType =					(uint16_t) bitmap_flip_byte(&fileheader[BF_TYPE], sizeof(ret.bfType));

	if(ret.bfType != (uint16_t)IDENTIFIER)
		return ret;

	ret.bfSize =					(uint32_t) bitmap_flip_byte(&fileheader[BF_SIZE], sizeof(ret.bfSize));
	ret.bfOffBits =			*(uint32_t*) &fileheader[BF_OFF_BITS];
	ret.biSize =				 *(uint32_t*) &fileheader[BI_SIZE];
	ret.biWidth =				*(int32_t*) &fileheader[BI_WIDTH];
	ret.biHeight =			 *(int32_t*) &fileheader[BI_HEIGHT];
	ret.biBitCount =		 *(uint16_t*) &fileheader[BI_BIT_COUNT];
	ret.biCompression =	 (uint32_t) bitmap_flip_byte(&fileheader[BI_COMPRESSION], sizeof(ret.biCompression));
	ret.biSizeImage =		*(uint32_t*) &fileheader[BI_SIZE_IMAGE];
	ret.biClrUsed =			 (uint32_t) bitmap_flip_byte(&fileheader[BI_CLR_USED], sizeof(ret.biClrUsed));
	ret.biClrImportant =	(uint32_t) bitmap_flip_byte(&fileheader[BI_CLR_IMPORTANT], sizeof(ret.biClrImportant));


	//Read to start of Pixel block
	//This block contains Colormasks and Colortables.
	ret.tablesc = ret.bfOffBits - read_counter;
	ret.tables = malloc(sizeof(char)* ret.tablesc);
	fread(ret.tables, sizeof(char), ret.tablesc, _file);
	//////////

	ret.error = 0;
	return ret;
}

static struct bitmap_image bitmap_read_pixel_data(FILE *_file, struct bitmap_file_header _header)
{
	uint32_t			**bitmap_buff;

	struct bitmap_image ret;

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
	for(int i = 0; i < ret.x; i++)
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

int bitmap_copy ( struct bitmap_image *_input, struct bitmap_image *_output ) {
	// TODO implement
	return 1;
}

int bitmap_convert_monochrome ( struct bitmap_image *_input, struct bitmap_image *_output ) {
	if ( !_input || !_output )
		return 1;

	uint8_t min_brightness = 0xff;
	uint8_t max_brightness = 0x00;
	uint8_t **monochrome_bitmap = (uint8_t**) dynalloc_2d_array( _input->x, _input->y, sizeof(uint8_t));

	for ( unsigned int x = 0; x < _input->x; x++ ) {
		for ( unsigned int y = 0; y < _input->y; y++ ) {
			monochrome_bitmap[x][y] = bitmap_rgb_luminance (
					_input->R[x][y],
					_input->G[x][y],
					_input->B[x][y] );

			if (monochrome_bitmap[x][y] > max_brightness)
				max_brightness = monochrome_bitmap[x][y];
			if (monochrome_bitmap[x][y] < min_brightness)
				min_brightness = monochrome_bitmap[x][y];
		}
	}

	_output->R = _output->G = _output->B = monochrome_bitmap;
	_output->tags = BITMAP_MONOCHROME;
	_output->x = _input->x;
	_output->y = _input->y;
	_output->monochrome_maximum_brightness = max_brightness;
	_output->monochrome_minimum_brightness = min_brightness;

	return 0;
}

int bitmap_shrink ( struct bitmap_image *_input, struct bitmap_image *_output, unsigned int _factor_x, unsigned int _factor_y ) {
	if ( !_input || !_output )
		return 1;

	/* New Size */
	_output->x = _input->x / _factor_x;
	_output->y = _input->y / _factor_y;
	_output->tags = _input->tags;

	/* Allocate memory */
	if ( _input->tags & BITMAP_MONOCHROME ) {
		_output->R = _output->G = _output->B =
			(uint8_t**) dynalloc_2d_array ( _output->x, _output->y, sizeof ( uint8_t) );
	} else {
		_output->R = (uint8_t**) dynalloc_2d_array ( _output->x, _output->y, sizeof ( uint8_t) );
		_output->G = (uint8_t**) dynalloc_2d_array ( _output->x, _output->y, sizeof ( uint8_t) );
		_output->B = (uint8_t**) dynalloc_2d_array ( _output->x, _output->y, sizeof ( uint8_t) );
	}

	for(unsigned int x = 0; x < _output->x; x++) {
		for(unsigned int y = 0; y < _output->y; y++) {
			// Unsafe for > 2^56 Pixels (Hopefully unrealistic)
			uint64_t color_sum[3] = {0,0,0};
			const uint64_t pixel_count = _factor_x * _factor_y;

			// Average Pixel block
			for(unsigned int row_c = 0; row_c < _factor_y; row_c++) {
				unsigned int row = y * _factor_y + row_c; //Offset

				for(unsigned int col_c = 0; col_c < _factor_x; col_c++) {
					unsigned int col = x * _factor_x + col_c; //Offset

					color_sum[0] += (uint64_t) _input->R[col][row];
					color_sum[1] += (uint64_t) _input->G[col][row];
					color_sum[2] += (uint64_t) _input->B[col][row];
				}//for col_c
			}//for row_c

			_output->R[x][y] = (uint8_t) (color_sum[0] / pixel_count);
			_output->G[x][y] = (uint8_t) (color_sum[1] / pixel_count);
			_output->B[x][y] = (uint8_t) (color_sum[2] / pixel_count);
		}//for y
	}//for x

	return 0;
}

int bitmap_fit_to_width ( struct bitmap_image *_input, struct bitmap_image *_output, unsigned int _width ) {
	unsigned int factor_x = (unsigned int)((float)_input->x / (float) _width );
	unsigned int factor_y = (unsigned int)(((float)_input->y / (float)_input->x ) * (float) factor_x * 2);

	return bitmap_shrink ( _input, _output, factor_x, factor_y );
}

static uint8_t bitmap_rgb_luminance(uint8_t R, uint8_t G, uint8_t B) {
	uint8_t ret;

	ret = sqrt( 0.299*pow(R,2) + 0.587*pow(G,2) + 0.114*pow(B,2) ); //(char)(R+R+B+G+G+G)/6;

	return ret;
}

char* bitmap_strerror( int _error ) {
	if ( _error >= bitmap_errors_cnt || _error < 0)
		return "Unknown Error.";

	return (char*) bitmap_errors[_error];
}
