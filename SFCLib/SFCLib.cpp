// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Rectangle.h"
#include "Point.h"
#include "SFCConversion.h"
#include "QueryBySFC.h"
#include "OutputSchema.h"
#include "SFCPipeline.h"

#include "tbb/task_scheduler_init.h"

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

int main(int argc, char* argv[])
{
	if (argc == 1) return 0;
	if (argc % 2 != 1) return 0; //attribute pair plus exe_name

	int nparallel = 0;
	int nsfc_type = 0;
	int nencode_type = 0;
	char szinput[256] = { 0 };//1.xyz
	char szoutput[256] = { 0 };

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-p") == 0)//if parallel
		{
			i++;
			nparallel = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-i") == 0)//input file path
		{
			i++;
			strcpy_s(szinput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)//output file path
		{
			i++;
			strcpy_s(szoutput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-s") == 0)//sfc conversion type
		{
			i++;
			nsfc_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-e") == 0)//output encoding type
		{
			i++;
			nencode_type = atoi(argv[i]);
		}
	}	 

	///////////////////////
	////pipeline
	double delta[3] = { 526000, 4333000, 300 };
	long  scale[3] = { 100, 100, 1000 };

	if (nparallel == 0)
	{
		if (strlen(szoutput) !=0 ) printf("serial run   "); //if not stdout ,print sth
		tbb::task_scheduler_init init_serial(1);
		run_pipeline<3, 20>(1, szinput, szoutput, 3000, nsfc_type, nencode_type, delta, scale);
	}

	if (nparallel == 1)
	{
		if (strlen(szoutput) != 0)  printf("parallel run "); //if not stdout ,print sth
		tbb::task_scheduler_init init_parallel;
		run_pipeline<3, 20>(init_parallel.default_num_threads(), szinput, szoutput, 3000, nsfc_type, nencode_type, delta, scale);
	}

	///////////////////////

	///////////////////////////////////////
	////2D case 8*8, i.e n=2, m=3
	//Point<long, 2> ptCoord; //SFC coordinates n=2
	//Point<long, 3> ptBits; //SFC bit sequence m=3

	//SFCConversion<2, 3> sfc;
	//OutputSchema<2, 3> trans;
	//	
	//for (int i = 0; i < 8; i++)
	//{
	//	for (int j = 0; j < 8; j++)
	//	{
	//		ptCoord[0] = j;//i
	//		ptCoord[1] = i;//j

	//		sfc.ptCoord = ptCoord;
	//		//sfc.MortonEncode();
	//		sfc.HilbertEncode();
	//		ptBits = sfc.ptBits;

	//		/*cout << i << ", " << j << "---";
	//		print_bits(i); cout << " ";
	//		print_bits(j); cout << " ---";
	//		print_bits(ptBits[0]); cout << " ";
	//		print_bits(ptBits[1]); cout << " ";
	//		print_bits(ptBits[2]);
	//		cout << endl;*/
	//		
	//		long outval = trans.BitSequence2Value(ptBits);

	//		cout << i << ", " << j << "====" << outval <<endl;
	//	}
	//}

	//
	//////////////////////////////////
	/////Butz's sample, n=5; m=4;
	//Point<long, 5> pt1;
	//pt1[0] = 10;//1010
	//pt1[1] = 11;//1011
	//pt1[2] = 3; //0011
	//pt1[3] = 13;//1101
	//pt1[4] = 5; //0101

	//Point<long, 4> pt2; //SFC bit sequence m=3

	//SFCConversion<5, 4> sfc2;
	//OutputSchema<5, 4> trans2;

	//sfc2.ptCoord = pt1;
	//sfc2.HilbertEncode();
	//pt2 = sfc2.ptBits;

	//long val = trans2.BitSequence2Value(pt2);

	//print_bits(val);

	//
	////2D sample
	//long Point1[2] = { 3, 2 };
	//long Point2[2] = { 6, 6 };
	//Point<long, 2> MinPoint(Point1);
	//Point<long, 2> MaxPoint(Point2);
	//Rectangle<long, 2> rec(MinPoint, MaxPoint);
	//QueryBySFC<long, 2, 3> querytest;
	//querytest.RangeQueryByBruteforce(rec, Morton);
	////querytest.RangeQueryByBruteforce(rec);
	//querytest.RangeQueryByRecursive(rec, Morton);

	//SFCConversion<2, 3> sfc2D;
	//OutputSchema<2, 3> trans2D;
	//sfc2D.ptCoord = Point1;
	//sfc2D.MortonEncode();	
	//string str32 = trans2D.BitSequence2String(sfc2D.ptBits, Base64);

	////3D sample
	//long Point31[3] = { 4, 2, 5 };
	//long Point32[3] = { 5, 4, 7 };
	//Point<long, 3> MinPoint3(Point31);
	//Point<long, 3> MaxPoint3(Point32);
	//Rectangle<long, 3> rec3(MinPoint3, MaxPoint3);
	//QueryBySFC<long, 3, 10> querytest3;
	//querytest3.RangeQueryByBruteforce(rec3, Morton);
	//querytest3.RangeQueryByRecursive(rec3, Morton);

	//SFCConversion<3, 9> sfc3D;
	//OutputSchema<3, 9> trans3D;
	//sfc3D.ptCoord = Point31;
	//sfc3D.MortonEncode();
	//string str3D = trans3D.BitSequence2String(sfc3D.ptBits, Base64);
	//Point<long, 9> mm = trans3D.String2BitSequence(str3D, Base64);
	////Point<long, 10> pttt = trans3D.String2BitSequence(str3D, Base64);

	//long Point333[3] = { 4, 2, 5 };
	//SFCConversion<3, 4> sfc3D3;
	//OutputSchema<3, 4> trans3D3;
	//sfc3D3.ptCoord = Point333;
	//sfc3D3.MortonEncode();
	//string str3D3 = trans3D3.BitSequence2String(sfc3D3.ptBits, Base32);
	//Point<long, 4> mmm = trans3D3.String2BitSequence(str3D3, Base32);


	

	//system("pause");
	return 0;
}

