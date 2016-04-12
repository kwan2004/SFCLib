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

template< typename T>
class OutputTransform
{

public:
	OutputTransform();
	~OutputTransform();
	

public:
	T bitSequence2Value(Point<T> ptOutput)
	{ 
		//size=m;bitLength=n;
		int size = ptOutput.returnSize();
		int bitLength = ptOutput.returnBitLength();
		int move = 0;
		T result = 0;
		for (int i = 0; i < size; i++)
		{
			move = bitLength*(size-i);
			result |= (ptOutput[i] << move);

		}
		return result;
	}

	string bitSequence2String(Point<T> ptOutput,StringType str_type)
	{
		//size=m;bitLength=n;
		int size = ptOutput.returnSize();
		//int bitLength = 0;
		int bitLength = ptOutput.returnBitLength();
		int totalBits=size*bitLength;
		T *temp_result = new T[1];
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
			T each_Result = *(temp_result + i);
			int times = totalBit_eachResult / baseNumber;
			mod = total%baseNumber;
			move = move + mod;
			T temp = *(temp_result + i);
			T *next_start = new T[1];
			for (int j = 0; j < times; j++)
			{
				if (j == 0 )
				{
					T result = temp;
					T mask = 1 << mod;
					mask = mask - 1;
				    *(next_start+i )= result&temp;
					*(next_start + i) = *(next_start + i) << (baseNumber - mod);
					temp >> move;
					move = move + 6;
				}
				else
				{
					T result = temp;
					T mask = 1 << Base;
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

	 Point<T> value2BitSequence(T value)
	{
		 Point<T, size> ptOutput;
		 int totalBits = size* bitLength;
		 for (int i = 0; i < size; i++)
		 {   
			 temp_value = value;
			 for (int j = 0; j < bitLength; j++)
			 {
				 move = bitLength*(size-i);
				 T mask = ((T)1 << move-1);
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