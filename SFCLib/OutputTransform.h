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
	long BitSequence2Value(Point<long, mBits> ptBits)
	{ 
		if (mBits * nDims >= 64) return 0;
		long result = 0;
		for (int i = 0; i < mBits; i++)
		{
			result |= (ptBits[i] << (mBits - i - 1)*nDims);
		}
		return result;
	}

	Point<long, mBits> Value2BitSequence(long value)
	{
		Point<long, mBits> ptOutput;
		if (mBits * nDims >= 64) return ptOutput;

		long mask = ((long)1 << nDims - 1);
		for (int i = 0; i < mBits; i++)
		{			
			ptOutput[mBits - i - 1] = (value >> (i*nDims)) & mask;
		}

		return ptOutput;
	}

	string BitSequence2String(Point<long, mBits> ptBits, StringType str_type)
	{
		int base = 0;
		if (str_type == Base32) base = 5;
		if (str_type == Base64) base = 6;

		int ntotalbits = mBits * nDims;

		//allocate the string space -----one more space for residual
		int nstrlen = (ntotalbits % base) ? (ntotalbits / base + 1) : (ntotalbits / base);
		char* szstr = new char[nstrlen + 1]; //last char for zero
		memset(szstr, 0, nstrlen + 1);
		int valid = strlen(szstr); //should be zero here

		//find the least common multiple of base and  nDims( lcm <= base*n)
		int nlcm = (base > nDims) ? base : nDims; 
		while (1)                       /* Always true. */
		{
			if (nlcm % base == 0 && nlcm % nDims == 0)
			{
				break;          /* while loop terminates. */
			}
			++nlcm;
		}

		//////compare m*n with lcm (lcm = t*n), so totalbits(m*n) can be divided into m/t blocks (block=lcm=t*n)
		int t = nlcm / nDims; //lcm is the bit number in one block; each block has t  nDims = t*n
		int nblock = (mBits % t) ? (mBits / t + 1) : (mBits / t); // m/t+1 or m/t---one more space for residual
				
		array<long, nblock>; //long is 64 bits, base64=6 bits, enough for 10 more dims---to contain t*n (t<=base)		
		for (int i = 0; i < nblock; i++)
		{
			///copy t*n to each block and get the decimal value
			long nblkvalue = 0;
			int nloops;
			if ((mBits % t) && ( nblock-1 == i)) //handle the final block
				nloops = mBits % t;
			else
				nloops = t;

			for (int j = 0; j < nloops; j++)
			{
				nblkvalue |= ptBits[mBits - i*nblock - j] << (j*nDims);
			}
			
			////change decimal value to character
			long ncharnum = nloops*nDims / base;
			unsigned int mask = ((unsigned int)1 << base) - 1;
			int nidx = 0;
			for (int j = 0; j < ncharnum; j++)
			{
				nidx = (nblkvalue >> (j*base)) & mask;
				if (str_type == Base64)
					*(szstr + (i*nblock + j)) = BASE64_TABLE_E2[nidx];
			}
		}

		string resultStr(szstr);
		delete[] szstr;

		return resultStr;	
	}

	Point<long, mBits> BitSequence2String(string szCode, StringType str_type)
	{
		Point<long, mBits> ptBits;

		int base = 0;
		if (str_type == Base32) base = 5;
		if (str_type == Base64) base = 6;

		int ntotalbits = mBits * nDims;
		int nstrlen = (ntotalbits % base) ? (ntotalbits / base + 1) : (ntotalbits / base);

		//find the least common multiple of base and  nDims( lcm <= base*n)
		int nlcm = (base > nDims) ? base : nDims;
		while (1)                       /* Always true. */
		{
			if (nlcm % base == 0 && nlcm % nDims == 0)
			{
				break;          /* while loop terminates. */
			}
			++nlcm;
		}

		int nstride = nlcm / base; //how many characters will be processed once
		unsigned int mask = ((unsigned int)1 << nDims) - 1;
		for (int i = 0; i < nstrlen; i += nstride)
		{
			int nloops = nstride;
			if (i + nstride > nstrlen)
				nloops = nstrlen - i;
			
			//convert nloops character to a long value
			int nvalue = 0;
			for (int j = 0; j < nloops; j++)
			{
				nvalue |= szCode.c_str[i*nstride +j] << base;
			}

			int num = nloops * base / nDims;
			for (int j = 0; j < num j++) //nloops character
			{
				ptBits[i*nstride + j] = (nvalue >> (nDims*j)) & mask;
			}
		}

		return ptBits;
	}	
};

#endif