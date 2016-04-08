#ifndef SFCCONERSION_H_
#define SFCCONERSION_H_

#include "Point.h"

template< int nDims,  int nBits>
class SFCConversion
{
public:
	Point<long, nDims) ptInput;
	Point<long, nBits) ptOutput;

	MortonEncode();
	MortonDecode();

	HilbertEncode();
	HilbertDecode();
};
#endif