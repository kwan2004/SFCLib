// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Rectangle.h"
#include "Point.h"
#include "SFCConversion.h"
#include "OutputTransform.h"
#include "QueryBySFC.h"

#include <iostream>
using namespace std;

void print_bits(unsigned int x)
{
	int i;
	for (i = 8 * sizeof(x) - 1; i >= 0; i--) // 8 * sizeof(x) - 1
	{
		(x & (1 << i)) ? putchar('1') : putchar('0');
	}
	//printf("\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	/////////////////////////////////////
	//2D case 8*8, i.e n=2, m=3
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

	
	////////////////////////////////
	///Butz's sample, n=5; m=4;
	Point<long, 5> pt1;
	pt1[0] = 10;//1010
	pt1[1] = 11;//1011
	pt1[2] = 3; //0011
	pt1[3] = 13;//1101
	pt1[4] = 5; //0101

	Point<long, 4> pt2; //SFC bit sequence m=3

	SFCConversion<5, 4> sfc2;
	OutputTransform<5, 4> trans2;

	sfc2.ptCoord = pt1;
	sfc2.HilbertEncode();
	pt2 = sfc2.ptBits;

	long val = trans2.bitSequence2Value(pt2);

	print_bits(val);


	///////////////////
	long Point1[3] = { 3, 4, 2};
	long Point2[3] = { 6, 5, 4};
	Point<long, 3> MinPoint(Point1);
	Point<long, 3> MaxPoint(Point2);
	Rectangle<long, 3> rec(MinPoint, MaxPoint);
	rec.generateComplateRec(MinPoint, MaxPoint);

	system("pause");
	return 0;
}

