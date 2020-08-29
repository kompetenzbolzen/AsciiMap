/*
 * src/dynalloc.h
 * (c) 2020 Jonas Gunz <himself@jonasgunz.de>
 * License: MIT
*/
#pragma once

#include <stdlib.h>

void** dynalloc_2d_array ( unsigned int _x, unsigned int _y, unsigned int _sizeof );

void dynalloc_2d_array_free ( unsigned int _x, unsigned int _y, void** _array );
