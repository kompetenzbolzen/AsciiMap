#include "m.h"

uint8_t avg(int argc, uint8_t *argv)
{
  uint8_t ret = 0;
  uint64_t sum = 0;

  for(int i = 0; i < argc; i++)
    sum += (uint64_t)argv[i];

  ret = (char)(sum / argc);

  return ret;
}//avg


float distance(uint8_t _1[3], uint8_t _2[3])
{
	return fabs(sqrt( pow((double)_1[0], 2) + pow((double)_1[1],2) + pow((double)_1[2],2) )
		 -  sqrt( pow((double)_2[0], 2) + pow((double)_2[1],2) + pow((double)_2[2],2) ) );
}
