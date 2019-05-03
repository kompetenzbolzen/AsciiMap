#include "character.h"

const char character_luminance_map [] = {' ', ' ', '.', ',', '`', '-', '~', '"', '*', ':', ';', '<', '!', '/', '?', '%', '&', '=', '$', '#'};
//const char map[] = {' ', '`', '.', ',', ':', ';', '\"', '+', '#', '@'};

char calc_char(uint8_t _c , uint8_t _min, uint8_t _max)
{
  float c = (float)(_c) / (_max - _min);
  return character_luminance_map [(int)((sizeof(character_luminance_map)-1) * (c))];
}
