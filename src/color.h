#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define _COLORS_SIZE	16u
#define COLOR_FG	1
#define COLOR_BG	2

struct console_color
{
	uint8_t R;
	uint8_t G;
	uint8_t B;

	char *no;
};

extern struct console_color colors[ _COLORS_SIZE ];

//Calculate luminance
//Order LSB first: BGR
uint8_t rgb_luminance(uint8_t R, uint8_t G, uint8_t B);

char *calc_col_ansi(uint8_t R, uint8_t G, uint8_t B, uint8_t _mode);
#endif //_COLOR_H_
