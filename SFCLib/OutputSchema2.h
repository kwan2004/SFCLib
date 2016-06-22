//#pragma once
#ifndef OUTPUTSCHEMA2_H_
#define OUTPUTSCHEMA2_H_

#include "Point.h"
#include <iostream>
#include <bitset>
using namespace std;

typedef enum
{
	Base32,
	Base64,
	Dec,
	Hex
} OutType;

static const char* const BASE32_TABLE_E2 = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
static const char* const BASE64_TABLE_E2 = "+/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz=";

template<int nDims, int  mBits>
class OutputSchema2
{
public:
	OutputSchema()
	{
		;
	}

public:
	
	string Value2String(uint256_t val, StringType str_type)
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
		int nlcm = CalculateLCM(base, nDims);

		//compare m*n with lcm (lcm = t*n), so totalbits(m*n) can be divided into m/t blocks (block=lcm=t*n)
		int t = nlcm / nDims; //lcm is the bit number in one block; each block has t  nDims = t*n
		int nblock = (mBits % t) ? (mBits / t + 1) : (mBits / t); // m/t+1 or m/t---one more space for residual

		//array<long, nblock>; //long is 64 bits, base64=6 bits, enough for 10 more dims---to contain t*n (t<=base)		
		int k = 0;
		for (int i = 0; i < nblock; i++)
		{
			//copy t*n to each block and get the decimal value
			long nblkvalue = 0;
			int nloops;
			if ((mBits % t) && (nblock - 1 == i)) //handle the final block
				nloops = mBits % t;
			else
				nloops = t;

			for (int j = 0; j < nloops; j++)
			{
				nblkvalue |= ptBits[i*t + j] << ((nloops - 1 - j)*nDims);
			}

			//change decimal value to character
			long ncharnum = (nloops*nDims % base) ? (nloops*nDims / base + 1) : (nloops*nDims / base);

			unsigned int mask = ((unsigned int)1 << base) - 1;
			int nidx = 0;

			for (int j = 0; j < ncharnum; j++)
			{
				nidx = (nblkvalue >> ((ncharnum - j - 1)*base)) & mask;
				if (str_type == Base64)
				{
					*(szstr + k) = BASE64_TABLE_E2[nidx];
					k++;
				}
				if (str_type == Base32)
				{
					*(szstr + k) = BASE32_TABLE_E2[nidx];
					k++;
				}
			}
		}

		string resultStr(szstr);
		delete[] szstr;
		return resultStr;
	}

	uint256_t String2Value(string szCode, StringType str_type)
	{
		Point<long, mBits> ptBits;

		int base = 0;
		if (str_type == Base32) base = 5;
		if (str_type == Base64) base = 6;

		// get value according to string
		int codeLength = szCode.length();
		long * codeBits = new long[codeLength];
		for (int i = 0; i < codeLength; i++)
		{
			char tt = szCode.c_str()[i];
			if (str_type == Base32)
			{
				for (int k = 0; k < 32; k++)
				{
					if (BASE32_TABLE_E2[k] == szCode.c_str()[i])
					{
						codeBits[i] = k;
						break;
					}
				}

			}
			else if (str_type == Base64)
			{
				for (int k = 0; k < 64; k++)
				{
					if (BASE64_TABLE_E2[k] == szCode.c_str()[i])
					{
						codeBits[i] = k;
						break;
					}
				}
			}
		}

		int ntotalbits = codeLength * base;

		//find the least common multiple of base and  nDims( lcm <= base*n)
		int nlcm = CalculateLCM(base, nDims);

		//compare codeLength*base with lcm (lcm = base*n), so totalbits(codeLength*base) can be divided into codeLengyh/t blocks (block=lcm=t*base)
		int t = (nlcm / base); //lcm is the bit number in one block; each block has t  nDims = t*n
		int nblock = (codeLength % t) ? (codeLength / t + 1) : (codeLength / t); // codeLength/t+1 or codeLength/t---one more space for residual

		//array<long, nblock>; //long is 64 bits, base64=6 bits, enough for 10 more dims---to contain t*n (t<=base)		
		int k = 0;
		for (int i = 0; i < nblock; i++)
		{
			///copy t*n to each block and get the decimal value
			long nblkvalue = 0;
			int nloops;
			if ((codeLength % t) && (nblock - 1 == i)) //handle the final block
				nloops = codeLength % t;
			else
				nloops = t;

			for (int j = 0; j < nloops; j++)
			{
				nblkvalue |= codeBits[i*t + j] << ((nloops - 1 - j)*base);
			}

			////change decimal value to m*n
			//long ncharnum = nloops*base / nDims;
			long ncharnum = 0;
			if (ntotalbits != mBits*nDims && i == nblock - 1)
			{
				ncharnum = nloops*base - (ntotalbits - mBits*nDims) / nDims;
			}
			else
			{
				ncharnum = nloops*base / nDims;
			}

			unsigned int mask = ((unsigned int)1 << nDims) - 1;
			int nidx = 0;
			for (int j = 0; j < ncharnum; j++)
			{
				ptBits[k] = (nblkvalue >> ((ncharnum - j - 1)*nDims)) & mask;
				k++;
			}
		}
		delete[] codeBits;
		return ptBits;
	}
};

#endif