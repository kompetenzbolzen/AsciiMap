/*
 * src/dynalloc.c
 * (c) 2020 Jonas Gunz <himself@jonasgunz.de>
 * License: MIT
*/
#include "dynalloc.h"

void** dynalloc_2d_array ( unsigned int _x, unsigned int _y, unsigned int _sizeof ) {
	void** ret = NULL;

	ret = malloc ( _x * sizeof ( void* ) );
	for ( int i = 0; i < _x; i++ )
		ret[i] = malloc ( _y * _sizeof );

	return ret;
}

void dynalloc_2d_array_free ( unsigned int _x, unsigned int _y, void** _array ) {
	for ( int i = 0; i < _x; i++ )
		free ( _array[i] );
	free(_array);
}
