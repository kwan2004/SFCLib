#pragma once
#ifndef OUTPUTTRANSFORM_H_
#define OUTPUTTRANSFORM_H_

#include "Point.h"
#include <iostream>
#include <bitset>
using namespace std;

typedef enum
{
	Base32,
	Base64,
} StringType;

static const char* const BASE64_TABLE_E2 = "+/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz=";

template<size,bitLength>
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
			move = bitLength*(size-i);
			result |= (ptOutput[i] << move);

		}
		return result;
	}

	string bitSequence2String(Point<long> ptOutput,StringType str_type)
	{
		//size=m;bitLength=n;
		int size = ptOutput.returnSize();
		//int bitLength = 0;
		int bitLength = ptOutput.returnBitLength();
		int totalBits=size*bitLength;
		long *temp_result = new long[1];
		int loopTimes = totalBits / 64;
		int move = 0;
		char *resultChar=NULL;
		if ((totalBits <= 64))
		{
			*(temp_result) = bitSequence2Value(ptOutput);
		}
		else
		{
			int number = 64 / size;
			for (int i = 0; i < loopTimes; i++)
			{
				for (int j = 0; j < size; j++)
				{
					move = size*(bitLength - j);
					*(temp_result + i) |= (*(temp_result + i*number + j) << move);
				}
			}
		}
		for (int i = 0; i < loopTimes; i++)
		{   
			int result=0;
			if (str_type == 1)
			{   
				int number = (sizeof(long) / 6);
				for (int j = 0; j <number ; j++)
				{
					long mask = 1 <<((number-1)*6) ;
					mask >> ((j - 1) * 6);
					result = (*(temp_result + i)&mask);
					result = result >> ((number - 1) * 6);
					*(resultChar + i*6+j) = BASE64_TABLE_E2[result];
				}
			}
		}

		string resultStr(resultChar);
		return resultStr;
	}

	 Point<long> Value2itSequence(long value)
	{
		 Point<long, size> ptOutput;
		 int totalBits = size* bitLength;
		 for (int i = 0; i < size; i++)
		 {   
			 temp_value = value;
			 for (int j = 0; j < bitLength; j++)
			 {
				 move = bitLength*(size-i);
				 long mask = ((long)1 << move-1);
				 temp_value = temp_value & mask;
				 move = bitLength*(size - i-1);
				 temp_value >> move;
				 ptOutput[i] = temp;
			 }
			 
		 }
		 return ptOutput;
	}

};

#endif