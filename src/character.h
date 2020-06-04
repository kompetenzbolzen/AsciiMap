#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include <stdint.h>
#include <string.h>

//Both maps produce very different results
extern const char *default_character_map;

//Select Char based on 1B brightness Value.
//if _custom_map == NULL default map is used
char calc_char(uint8_t _c, uint8_t _min, uint8_t _max, char *_custom_map);

#endif //_CHARACTER_H_
