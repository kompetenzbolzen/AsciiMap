#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "m.h"

#define _COLORS_SIZE 16u

struct console_color
{
	uint8_t R;
	uint8_t G;
	uint8_t B;

	char *no;
};

struct console_color colors[ _COLORS_SIZE ];

//Calculate luminance
//Order LSB first: BGR
uint8_t rgb_avg(uint8_t R, uint8_t G, uint8_t B);

//Get nearest printable color in console
char *calc_col(uint8_t R, uint8_t G, uint8_t B);

char *calc_col_ansi(uint8_t R, uint8_t G, uint8_t B);

#endif //_COLOR_H_
