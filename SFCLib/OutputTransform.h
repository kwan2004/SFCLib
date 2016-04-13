//#pragma once
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

template<int nDims, int  mBits>
class OutputTransform
{

public:
	OutputTransform()
	{
		;
	}
	~OutputTransform()
	{
		;
	}
	

public:
	long bitSequence2Value(Point<long, mBits> ptBits)
	{ 
		long result = 0;
		for (int i = 0; i < mBits; i++)
		{
			result |= (ptBits[i] << (mBits - i - 1)*nDims);
		}
		return result;
	}

	Point<long, mBits> Value2itSequence(long value)
	{
		Point<long, mBits> ptOutput;

		int totalBits = nDims* mBits;
		for (int i = 0; i < mBits; i++)
		{
			temp_value = value;
			for (int j = 0; j <nDims; j++)
			{
				move = nDims*(mBits - i);
				long mask = ((long)1 << move - 1);
				temp_value = temp_value & mask;
				move = nDims*(mBits - i - 1);
				temp_value >> move;
				ptOutput[i] = temp;
			}

		}
		return ptOutput;
	}


	string bitSequence2String(Point<long, mBits> ptBits, StringType str_type)
	{
		//size=m;bitLength=n;
		int size = ptBits.returnSize();
		//int bitLength = 0;
		int bitLength = ptBits.returnBitLength();
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
				for (int j = 0; j < number; j++)
				{
					move = bitLength*(number - j);
					*(temp_result + i) |= (ptOutput[i*number + j]<< move);
				}
			}
		}

		int baseNumber = 0;
		if (str_type == 0)baseNumber = 5;
		if (str_type == 0)baseNumber = 6;
		int mod = 0;
		int totalBit_eachResult = (64 / m)*bitLength;
		int move = 0;
		for (int i = 0; i < loopTimes; i++)
		{   
			//int result=0;
			long each_Result = *(temp_result + i);
			int times = totalBit_eachResult / baseNumber;
			mod = total%baseNumber;
			move = move + mod;
			long temp = *(temp_result + i);
			long *next_start = new long[1];
			for (int j = 0; j < times; j++)
			{
				if (j == 0 )
				{
					long result = temp;
					long mask = 1 << mod;
					mask = mask - 1;
				    *(next_start+i )= result&temp;
					*(next_start + i) = *(next_start + i) << (baseNumber - mod);
					temp >> move;
					move = move + 6;
				}
				else
				{
					long result = temp;
					long mask = 1 << Base;
					mask = mask - 1;
					result = result & mask;
					*(resultChar + i * 6 + times - j) = BASE64_TABLE_E2[result];
					if (i != 0&&(j==times-1))
					{
						result = *(next_start + i - 1)&result;
						*(resultChar + i * 6 + times - j)=BASE64_TABLE_E2[result];
					}
					temp >> move;
					move = move + 6;
				}

			}
			totalBit_eachResult = totalBit_eachResult + mod;
			/*	int number1 = (sizeof(long) / 6);
				for (int j = 0; j <number1 ; j++)
				{
					long mask = 1 <<((number1-1)*6) ;
					mask >> ((j - 1) * 6);
					result = (*(temp_result + i)&mask);
					result = result >> ((number1 - 1) * 6);
					*(resultChar + i*6+j) = BASE64_TABLE_E2[result];
				}*/


		}

		string resultStr(resultChar);
		return resultStr;
	}
	
};

#endif