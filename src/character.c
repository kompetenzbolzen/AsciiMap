#include "character.h"

const char *default_character_map="  .,-~\"*:;<!/?%&=$#";
//const char *default_character_map=" `.,:;/+#@";

char calc_char(uint8_t _c , uint8_t _min, uint8_t _max, char *_custom_map) {
	const char *map = _custom_map == 0 ? default_character_map : _custom_map;

	float c = (float)(_c) / (_max - _min);
	return map [(int)( strlen( map ) * (c))];
}

char* calc_col_ansi(uint8_t R, uint8_t G, uint8_t B, uint8_t _mode) {
	int mode = _mode == COLOR_BG ? 4 : 3;
	int num = 36 * (R/51) + 6 * (G/51) + (B/51);
	char *c = malloc(12);
	snprintf( c, 8, "\e[%i8;5;", mode );
	snprintf( c + 7, 5, "%im", num + 16 );

	return c;
}
