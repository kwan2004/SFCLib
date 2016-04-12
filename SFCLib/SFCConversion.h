#pragma once
#ifndef SFCCONERSION_H_
#define SFCCONERSION_H_

#include "Point.h"

template< int nDims,  int  mBits>
class SFCConversion
{
public:
	Point<long, nDims> ptInput; //n*m
	Point<long,  mBits> ptOutput; //m*n

	void MortonEncode(void) // from n*m to m*n
	{
		for (int i = 0; i <  mBits; i++)//m
		{
			ptOutput[i] = 0;
			long mask = 1 << ( mBits - i);
			for (int j = 0; j < nDims; j++) //get one bit from each dim
			{
				if (ptInput[j] & mask) // both 1
					ptOutput[i] |= 1 << (nDims - j);// dim iteration
			}//
		}//m group
		//set the ouput point n;
		ptOutput.getBitLength(nDims);
	}
	void MortonDecode(void)
	{
		for (int i = 0; i <  mBits; i++)//m n-bits
		{			
			long ntemp = ptOutput[i]; // each row in  m n-bits
			long mask = 1 << ( mBits - i);
			mask = mask - 1;
			for (int j = 0; j < nDims; j++) 
			{
				if (ntemp & 1) //get the last bit from  each bits row 
					ptInput[nDims-j-1] |= mask;
				
				ntemp >>= 1; //move forward one bit
			}//
		}//n dim
	}

	void HilbertEncode(void);
	void HilbertDecode(void);
};



#endif