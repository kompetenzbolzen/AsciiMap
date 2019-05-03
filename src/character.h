#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include <stdint.h>

//Both maps produce very different results
const char character_luminance_map [] ;

//Select Char based on 1B brightness Value
char calc_char(uint8_t _c, uint8_t _min, uint8_t _max);

#endif //_CHARACTER_H_
