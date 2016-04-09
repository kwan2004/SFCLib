#pragma once
#ifndef OUTPUTTRANSFORM_H_
#define OUTPUTTRANSFORM_H_
#include "Point.h"
#include <iostream>
using namespace std;

enum StringType
{
	Base32,
	Base64,
};

static const char* const BASE64_TABLE_E2 = "+/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz=";
class OutputTransform
{

public:
	OutputTransform();
	~OutputTransform();
	

public:
	long bitSequence2Value(Point<long> ptOutput)
	{ 
		//size=m;bitLength=n;
		int size = ptOutput.returnSize();
		int bitLength = ptOutput.returnBitLength();
		int move = 0;
		long result = 0;
		for (int i = 0; i < size; i++)
		{
			move = size*(bitLength - i);
			result |= (ptOutput[i] << move);
		}
		return result;
	}

	string bitSequence2String(Point<long> ptOutput,StringType)
	{
		//size=m;bitLength=n;
		int size = ptOutput.returnSize();
		int bitLength = ptOutput.returnBitLength();
	}


};

#endif