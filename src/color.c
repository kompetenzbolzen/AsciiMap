#include "color.h"

struct console_color colors[] = { //Standard VGA colors
	{0, 0, 0,	"30"}, //Black
	{170, 0, 0,	"31"}, //red
	{0, 170, 0, 	"32"}, //Green
	{170, 85, 0,	"33"}, //Brown
	{0, 0, 170, 	"34"}, //blue
	{170, 0, 170, 	"35"}, //Magenta
	{0, 170, 170, 	"36"}, //Cyan
	{170,170,170,	"37"}, //Grey
	{85,85,85, 	"30;1"},
	{255,85,85, 	"31;1"},
	{85,255,85, 	"32;1"},
	{255,255,85, 	"33;1"},
	{85,85,255, 	"34;1"},
	{255,85,255,	"35;1"},
	{85,255,255,	"36;1"},
	{255, 255, 255,	"37;1"}
};

uint8_t rgb_luminance(uint8_t R, uint8_t G, uint8_t B)
{
	uint8_t ret;

	ret = sqrt( 0.299*pow(R,2) + 0.587*pow(G,2) + 0.114*pow(B,2) ); //(char)(R+R+B+G+G+G)/6;

	return ret;
}

char* calc_col_ansi(uint8_t R, uint8_t G, uint8_t B, uint8_t _mode)
{
	int mode = _mode == COLOR_BG ? 4 : 3;
	int num = 36 * (R/51) + 6 * (G/51) + (B/51);
	char *c = malloc(12);
	snprintf( c, 8, "\e[%i8;5;", mode );
	snprintf( c + 7, 5, "%im", num + 16 );

	return c;
}
