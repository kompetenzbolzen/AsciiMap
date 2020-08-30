#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_FG	1
#define COLOR_BG	2

//Both maps produce very different results
extern const char *default_character_map;

//Select Char based on 1B brightness Value.
//if _custom_map == NULL default map is used
char calc_char(uint8_t _c, uint8_t _min, uint8_t _max, char *_custom_map);

char *calc_col_ansi(uint8_t R, uint8_t G, uint8_t B, uint8_t _mode);

#endif //_CHARACTER_H_
