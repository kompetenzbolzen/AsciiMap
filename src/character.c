#include "character.h"

const char *default_character_map="  .,-~\"*:;<!/?%&=$#";
//const char *default_character_map=" `.,:;/+#@";

char calc_char(uint8_t _c , uint8_t _min, uint8_t _max, char *_custom_map)
{
	const char *map = _custom_map == 0 ? default_character_map : _custom_map;

	float c = (float)(_c) / (_max - _min);
	return map [(int)( strlen( map ) * (c))];
}
