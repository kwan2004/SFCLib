// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Point.h"
#include "SFCConversion.h"
#include "OutputTransform.h"

#include <iostream>
using namespace std;

void print_bits(unsigned int x)
{
	int i;
	for (i =3 ; i >= 0; i--) // 8 * sizeof(x) - 1
	{
		(x & (1 << i)) ? putchar('1') : putchar('0');
	}
	//printf("\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	Point<long, 2> ptCoord; //SFC coordinates n=2
	Point<long, 3> ptBits; //SFC bit sequence m=3

	SFCConversion<2, 3> sfc;
	OutputTransform<2, 3> trans;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ptCoord[0] = j;//i
			ptCoord[1] = i;//j

			sfc.ptCoord = ptCoord;
			//sfc.MortonEncode();
			sfc.HilbertEncode();
			ptBits = sfc.ptBits;

			/*cout << i << ", " << j << "---";
			print_bits(i); cout << " ";
			print_bits(j); cout << " ---";
			print_bits(ptBits[0]); cout << " ";
			print_bits(ptBits[1]); cout << " ";
			print_bits(ptBits[2]);
			cout << endl;*/
			
			long outval = trans.bitSequence2Value(ptBits);

			cout << i << ", " << j << "====" << outval <<endl;
		}
	}

	return 0;
}

