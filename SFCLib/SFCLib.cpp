// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"

#include "SFCConversion.h"
//#include "OutputSchema.h"
#include "QueryBySFC.h"

#include "SFCPipeline.h"

#include "SFCConversion2.h"
#include "OutputSchema2.h"

#include "RandomLOD.h"

//#include "tbb/task_scheduler_init.h"

#include <iostream>
#include <fstream>
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

void print_ranges(char * str, vector<sfc_bigint>& ranges)
{
	long long ntotal_len = 0;
	if (str == NULL) return;

	//printf("%s \n", str);
	cout << str << endl;
	for (int i = 0; i < ranges.size(); i = i + 2)
	{
		//printf("\n");

		//printf("%lld---%lld\n", ranges[i], ranges[i + 1]);
		cout << ranges[i] << "----" << ranges[i + 1] <<endl;

		//ntotal_len += (long long )(ranges[i + 1] - ranges[i] + 1);		
	}

	//cout << "total len:  " << ntotal_len << endl;
}

void print_ranges_str(char * str, vector<string>& ranges)
{
	if (str == NULL) return;

	printf("%s \n", str);
	for (int i = 0; i < ranges.size(); i = i + 2)
	{
		//printf("\n");

		printf("%s---%s\n", ranges[i].c_str(), ranges[i + 1].c_str());

	}
}

int main(int argc, char* argv[])
{
	
	//if (argc == 1) return 0;
	//if (argc % 2 != 1) return 0; //attribute pair plus exe_name	

#ifdef PARALLEL_PIPELINE
	//here the SFCGen tool

	const int ndims = 3;
	const int mbits = 30;

	//-p 0 -s 1 -e 2 -t ct.txt -l 10 -i ahn2.txt -o ee.txt 
	int nparallel = 0;

	int nsfc_type = 0;
	int nencode_type = 0;

	bool bisonlysfc = false;

	int nitem_num = 5000;

	bool bislod = false;
	int lod_levels = 0;

	char szinput[256] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char sztransfile[256] = { 0 };

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-p") == 0)//if parallel: 0 sequential, 1 max parallel
		{
			i++;
			nparallel = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-i") == 0)//input file path
		{
			i++;
			strcpy(szinput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)//output file path
		{
			i++;
			strcpy(szoutput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-s") == 0)//sfc conversion type: 0 morthon, 1 hilbert
		{
			i++;
			nsfc_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-e") == 0)//output encoding type: 0 number 1 base32 2 base64
		{
			i++;
			nencode_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-t") == 0)//coordinates transformation file, two lines: translation and scale, comma separated
		{
			i++;
			strcpy(sztransfile, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-onlysfc") == 0)//output onlye sfc code
		{
			i++;
			bisonlysfc = true; //(bool)atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-l") == 0)//if generate the lod value and the lod levels
		{
			i++;
			bislod = true;
			lod_levels = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-n") == 0)//if points number per chunk during parallel
		{
			i++;
			nitem_num = atoi(argv[i]);
			continue;
		}
	}	 

	///////////////////////////////////////////////////
	///get the coordinates transfomration file--one more for lod value
	double delta[ndims + 1] = { 0 }; // 526000, 4333000, 300
	long  scale[ndims + 1] = { 1 }; //100, 100, 1000

	for (int i = 1; i < ndims + 1; i++)
	{
		delta[i] = 0;
		scale[i] = 1;
	}

	if (strlen(sztransfile) != 0)
	{
		FILE* input_file = NULL;
		input_file= fopen(sztransfile, "r");
		if (input_file)
		{
			int j;
			char buf[1024];
			char * pch, *lastpos;
			char ele[64];

			//////translation
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 64);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				delta[j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			delta[j] = atof(lastpos); //final part

			//////scale
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 64);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				scale[j] = atoi(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			scale[j] = atoi(lastpos); //final part

			fclose(input_file);
		}//end if input_file
	}//end if strlen
	
	/////////////////////////////////
	////pipeline
	if (nparallel == 0)
	{
		if (strlen(szoutput) !=0 ) printf("serial run   "); //if not stdout ,print sth
		tbb::task_scheduler_init init_serial(1);
		
		if (bislod)//lod value, one more dimension
			run_pipeline<ndims+1, mbits>(1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
		else
			run_pipeline<ndims, mbits>(1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);

	}

	if (nparallel == 1)
	{
		if (strlen(szoutput) != 0)  printf("parallel run "); //if not stdout ,print sth
		tbb::task_scheduler_init init_parallel(tbb::task_scheduler_init::automatic);

		if (bislod)//lod value, one more dimension
			run_pipeline<ndims+1, mbits>(init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
			nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
		else
			run_pipeline<ndims, mbits>(init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
			nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
	}
	
#endif
	///////////////////////

#ifdef SFC_GEN_TEST
	///here just for unit tests of SFC code calculation and query
	///3D case
	Point<long, 3> pt3d; //SFC coordinates n=2

	SFCConversion2<3, 3> sfc3d;
	SFCConversion<3, 3> sfc3d_old;
	//OutputSchema2<2, 4> trans;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				pt3d[0] = i;//i
				pt3d[1] = j;//j
				pt3d[2] = k;//k

				sfc_bigint outval = sfc3d.HilbertEncode(pt3d);	
				pt3d = sfc3d.HilbertDecode(outval);
				cout << i << ", " << j << ", " << k << "---M-->" << outval << "----->" << pt3d[0] << ", " << pt3d[1] << ", " <<  pt3d[2]<<endl; //
			}
		}
	}
	cout << "-----------------------------------" << endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				pt3d[0] = i;//i
				pt3d[1] = j;//j
				pt3d[2] = k;//k

				sfc_bigint outval_old = sfc3d_old.HilbertEncode(pt3d);
				pt3d = sfc3d_old.HilbertDecode(outval_old);
				cout << i << ", " << j << ", " << k << "---L-->" << outval_old << "----->" << pt3d[0] << ", " << pt3d[1] << ", " << pt3d[2] << endl;
			}
		}
	}

	pt3d = sfc3d_old.HilbertDecode(511);
	long long  aaaaa = (long long)sfc3d_old.HilbertEncode(pt3d);

	/////////////////////////////////////////////////////
	///true 4D data 85999.4,446266,-1.65,9,651295384353375995169439
	Point<double, 4> pt1;
	pt1[0] = 85999.42;//1010
	pt1[1] = 446266.47;//1011
	pt1[2] = -1.65; //0011
	pt1[3] = 9;//1101

	Point<double, 4> pt2;
	pt2[0] = 85098.38;//1010
	pt2[1] = 446440.06;//1011
	pt2[2] = 18.34; //0011
	pt2[3] = 9;//1101

	double delta[4] = { 80000.00, 437500.00, -20.0, 0.0 }; // 526000, 4333000, 300
	long  scale[4] = { 100, 100, 100, 1 }; //100, 100, 1000

	CoordTransform<double, long, 4> cotrans;
	cotrans.SetTransform(delta, scale);

	Point<long, 4> MinPt2 = cotrans.Transform(pt1);
	Point<long, 4> MaxPt2 = cotrans.Transform(pt2);

	SFCConversion<4, 30> sfctest;
	OutputSchema2<4, 30> transtest;

	sfc_bigint p1 = sfctest.HilbertEncode(MinPt2);
	cout << p1 << endl;
	cout << transtest.Value2String(p1, Base64).c_str() << endl;

	Point<long, 4> pt0 = sfctest.HilbertDecode(p1);

	sfc_bigint p2 = sfctest.HilbertEncode(MaxPt2);
	string stra = transtest.Value2String(p2, Base64).c_str();
	cout << p2 << endl;
	cout << stra << "  "<< p2.str()<< endl;

	string res= stra;//85098.38 446440.06 38.34 9

	SFCConversion<4, 30> sfctest2;
	OutputSchema2<4, 30> transtest2;

	sfc_bigint p3 = transtest2.String2Value(res, Base64);
	cout << p3 << endl;
	if (p2 == p3)
	{
		int a = 0;
	}
	Point<long, 4> Pt3 = sfctest2.HilbertDecode(p3);

	long long aq = 2349505055;
	string aa = transtest.Value2String(aq, Base64);
	cout << aa << endl;
	sfc_bigint bq = transtest2.String2Value(aa, Base64);
	cout << bq << endl;

	///////////////////////////////////////
	////2D case 16*16, i.e n=2, m=4
	Point<long, 2> ptCoord; //SFC coordinates n=2

	SFCConversion2<2, 4> sfc;
	OutputSchema2<2, 4> trans;

	SFCConversion<2, 4> sfc_old;
	//OutputSchema2<2, 4> trans;

	int a, b;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			ptCoord[0] = i;//i
			ptCoord[1] = j;//j

			sfc_bigint outval = sfc.HilbertEncode(ptCoord);

			ptCoord[0] = i;//i
			ptCoord[1] = j;//j

			sfc_bigint outval_old = sfc_old.HilbertEncode(ptCoord);

			/*cout << i << ", " << j << "---M-->" << outval <<  "  ,  " << trans.Value2String(outval, Base64); //<< endl
			
			Point<long, 2> pt2d = sfc.HilbertDecode(outval);

			a = pt2d[0];
			b = pt2d[1];
			cout  << "--->" << a << " , " << b << endl;*/

			
			cout << i << ", " << j << "---L-->" << outval_old << "  ,  " << trans.Value2String(outval_old, Base64); //<< endl

			Point<long, 2>  pt2d = sfc_old.HilbertDecode(outval_old);
			a = pt2d[0];
			b = pt2d[1];
			cout << "--->" << a << " , " << b << endl;
		}
	}

	SFCConversion<4, 6> sfc_a;
	SFCConversion2<4, 6> sfc_b;
	Point<long, 4> pta = sfc_a.HilbertDecode(476);
	Point<long, 4> ptb = sfc_b.HilbertDecode(476);

	//////////////////////////////////
	/////Butz's sample, n=5; m=4;
	Point<long, 5> pt3;
	pt3[0] = 10;//1010
	pt3[1] = 11;//1011
	pt3[2] = 3; //0011
	pt3[3] = 13;//1101
	pt3[4] = 5; //0101

	SFCConversion<5, 30> sfc2_old;
	sfc_bigint val_old = sfc2_old.HilbertEncode(pt3);
	//Point<long, 6> pt4; //SFC bit sequence m=3

	pt3[0] = 5;//1010
	pt3[1] = 13;//1011
	pt3[2] = 3; //0011
	pt3[3] = 11;//1101
	pt3[4] = 10; //0101

	SFCConversion2<5, 30> sfc2;
	sfc_bigint val = sfc2.HilbertEncode(pt3);	

	cout << "butz--" << val << "," << val_old << endl;
	//print_bits(val);

	Point<long, 5> pt4 = sfc2_old.HilbertDecode(val_old);
	Point<long, 5> pt5 = sfc2.HilbertDecode(val);

	///////////////////////////////////////////////////////////////
	///performance comparison between JKLawder and Doug Moore-- both are based on Butz bits operation
	///Moore: 272s ; lawders: 14s
	tbb::tick_count t0 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		sfc2_old.HilbertEncode(pt3);
	}
	tbb::tick_count t1 = tbb::tick_count::now();
	cout << "Laweder's hilbert time = " <<(t1 - t0).seconds()<<endl;

	//////////
	t0 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		sfc2_old.MortonEncode(pt3);
	}
	t1 = tbb::tick_count::now();
	cout << "Laweder's morton time = " << (t1 - t0).seconds() << endl;

	///////////////
	t0 = tbb::tick_count::now();

	for (int i = 0; i < 1000000; i++)
	{
		sfc2.HilbertEncode(pt3);
	}
	t1 = tbb::tick_count::now();
	cout << "Moore's time = " << (t1 - t0).seconds() << endl;
#endif
#ifdef SFC_QUERY_TEST
	///////////////////////////////////
	////2D sample--range query
	long Point1[2] = { 3, 2}; //3, 2 //8, 4
	long Point2[2] = { 4, 14 };  //5, 5//12, 9
	Point<long, 2> MinPoint(Point1);
	Point<long, 2> MaxPoint(Point2);
	Rect<long, 2> rec(MinPoint, MaxPoint);
	QueryBySFC<long, 2, 4> querytest;
	vector<sfc_bigint> vec_res = querytest.RangeQueryByBruteforce_LNG(rec, Hilbert);
	print_ranges("hilbert 2d brute force", vec_res);

	vector<sfc_bigint> vec_res2 = querytest.RangeQueryByRecursive_LNG(rec, Hilbert, 0);
	print_ranges("hilbert 2d recursive", vec_res2);

	/*vector<string> vec_res5 = querytest.RangeQueryByBruteforce_STR(rec, Hilbert, Base64);
	print_ranges_str("hilbert 2d brute force", vec_res5);

	vector<string> vec_res6 = querytest.RangeQueryByRecursive_STR(rec, Hilbert, Base64, 0);
	print_ranges_str("hilbert 2d recursive", vec_res6);*/

	///////////////////////////////////
	////3D sample----range query
	long Point31[4] = { 6, 2, 15, 6 };//
	long Point32[4] = { 35, 38, 17, 28 };//

	//long Point31[3] = { 8, 8, 16 };//,6
	//long Point32[3] = { 15, 15, 23 };//, 28 

	SFCConversion<4, 18> sfc_3d;

	Point<long, 4> MinPoint3(Point31);
	Point<long, 4> MaxPoint3(Point32);

	Rect<long, 4> rec3(MinPoint3, MaxPoint3);

	QueryBySFC<long, 4, 8> querytest3;
	vector<sfc_bigint> vec_res3 = querytest3.RangeQueryByBruteforce_LNG(rec3, Hilbert);
	print_ranges("Hilbert 3d brute force", vec_res3);

	vector<sfc_bigint> vec_res4 = querytest3.RangeQueryByRecursive_LNG(rec3, Hilbert, 0);
	print_ranges("Hilbert 3d recursive", vec_res4);
	

	/*vector<string> vec_res7 = querytest3.RangeQueryByBruteforce_STR(rec3, Hilbert, Base64);
	print_ranges_str("hilbert 3d brute force", vec_res7);

	vector<string> vec_res8 = querytest3.RangeQueryByRecursive_STR(rec3, Hilbert, Base64, 0);
	print_ranges_str("hilbert 3d recursive", vec_res8);*/

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
	
#endif

#ifdef SFC_QUERY
	//here the SFCQuery tool
	const int ndims = 4;
	const int mbits = 30;

	///85999.42,446266.47,-1.65,9,651295384353375995169439
	//-i 85999.0/85999.5/446266/446266.5/-2.0/-1.5/8/9 -s 1 -e 0 -t ct.txt -n 1000 -o qq3.sql

	//85999.1,446250.23,-1.69,9,651295397912973650169147
	//-i 85999.0/85999.5/446250/446250.4/-2.0/-1.5/8/9 -s 1 -e 0 -t ct.txt -n 0 -o qq5.sql
	//-i 85545.3000/85695.3000/446465.6500/446615.6500/-99999999.0000/-99999999.0000/-99999999.0000/-99999999.0000 -s 1 -e 0 -t ct.txt -n 0 -o qq5.sql
	//-i 85545.3000/85695.3000/446465.6500/446615.6500/-2.0000/-1.5000/8.0000/9.0000 -s 1 -e 0 -t ct.txt -n 0 -o qq5.sql
	int nsfc_type = 0;
	int nencode_type = 0;

	bool bisonlysfc = false;

	bool bislod = false;
	int lod_levels = 0;
	int nranges = 0; //if nranges =0; means search to the bottom level

	char szinput[1024] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char sztransfile[256] = { 0 };

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0)//input filter coordinates
		{
			i++;
			strcpy(szinput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)//output file path
		{
			i++;
			strcpy(szoutput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-s") == 0)//sfc conversion type: 0 morthon, 1 hilbert
		{
			i++;
			nsfc_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-e") == 0)//output encoding type: 0 number 1 base32 2 base64
		{
			i++;
			nencode_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-t") == 0)//coordinates transformation file, two lines: translation and scale, comma separated
		{
			i++;
			strcpy(sztransfile, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-n") == 0)//number of return ranges
		{
			i++;
			nranges = atoi(argv[i]);
			continue;
		}
	}

	///////////////////////////////////////////////////
	///get the coordinates transfomration file--one more for lod value
	double delta[ndims + 1] = { 0 }; // 526000, 4333000, 300
	long  scale[ndims + 1] = { 1 }; //100, 100, 1000

	for (int i = 1; i < ndims + 1; i++)
	{
		delta[i] = 0;
		scale[i] = 1;
	}

	if (strlen(sztransfile) != 0)
	{
		FILE* input_file = NULL;
		input_file = fopen(sztransfile, "r");
		if (input_file)
		{
			int j;
			char buf[1024];
			char * pch, *lastpos;
			char ele[128];

			//////translation
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 128);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				delta[j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			delta[j] = atof(lastpos); //final part

			//////scale
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 128);
				strncpy(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				scale[j] = atoi(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			scale[j] = atoi(lastpos); //final part

			fclose(input_file);
		}//end if input_file
	}//end if strlen
	
	CoordTransform<double, long, ndims> cotrans;
	cotrans.SetTransform(delta, scale);
	////////////////////////////////////////////////
	//get the input filter
	double pt1[ndims] = { 0.0f };
	double pt2[ndims] = { 0.0f };

	unsigned int dim_valid[ndims] = { 0 };

	memset(pt1, 0, sizeof(double)*ndims);
	memset(pt2, 0, sizeof(double)*ndims);
	memset(dim_valid, 0, sizeof(unsigned int)*ndims);

	char * pch, *lastpos;
	char ele[128];

	lastpos = szinput;
	for (int i = 0; i < ndims; i++)
	{
		///////min
		memset(ele, 0, 128);
		
		pch = strchr(lastpos, '//');
		strncpy(ele, lastpos, pch - lastpos);
		
		if (strcmp(ele, "-99999999.0000") != 0)//if "-99999999.0000", not set
		{
			pt1[i] = atof(ele);
			dim_valid[i] = 1;
		}
		else
		{ 
			pt1[i] = 0; ///this min value is not set,just assign 0
		}

		lastpos = pch + 1;
		///////max
		if (i != ndims - 1)
		{
			memset(ele, 0, 128);

			pch = strchr(lastpos, '//');
			strncpy(ele, lastpos, pch - lastpos);
			
			if (strcmp(ele, "-99999999.0000") != 0) //if "-99999999.0000", not set 
			{
				pt2[i] = atof(ele);
				dim_valid[i] = 1;
			}
			else
			{
				pt2[i] = 0; ///this max value is not set,just assign 2^mbits -11 <  < mbits - 1
			}

			lastpos = pch + 1;
		}
		else
		{
			if (strcmp(lastpos, "-99999999.0000") != 0) //if "-99999999.0000", not set
			{ 
				pt2[i] = atof(lastpos);
				dim_valid[i] = 1;
			}
			else
			{
				pt2[i] = 0; ///this max value is not set,just assign 2^mbits -1 1 << mbits - 1
			}
		}

	}
	///////////////////////////////////////////////
	//point transfomration
	Point<double, ndims> MinPt1(pt1);
	Point<double, ndims> MaxPt1(pt2);

	Point<long, ndims> MinPt2 = cotrans.Transform(MinPt1);
	Point<long, ndims> MaxPt2 = cotrans.Transform(MaxPt1);

	///to check if any dim is not set
	for (int i = 0; i < ndims; i++)
	{
		if (dim_valid[i] ==0 )// this dim is not set
		{
			MinPt2[i] = 0;
			MaxPt2[i] = 1 << mbits - 1;
		}
	}

	/////////////////////////////////////////////////////
	////query
	Rect<long, ndims> rec(MinPt2, MaxPt2);
	QueryBySFC<long , ndims, mbits> querytest;

	std::ostream* out_s;
	ofstream range_file;
	if (strlen(szoutput) != 0)
	{
		range_file.open(szoutput);
		out_s = &range_file;
	}
	else
	{ 
		out_s = &cout;
	}

	tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());

	tbb::tick_count t0 = tbb::tick_count::now();

	if (nencode_type == 0) //number
	{
		/*vector<sfc_bigint> vec_res2 = querytest.RangeQueryByBruteforce_LNG(rec, (SFCType)nsfc_type);*/

		vector<sfc_bigint> vec_res2 = querytest.RangeQueryByRecursive_LNG_P(rec, (SFCType)nsfc_type, nranges);
		//print_ranges("hilbert 2d recursive", vec_res2);

		for (int i = 0; i < vec_res2.size(); i = i + 2)
		{
			//fprintf(output_file, "%lld,%lld\n", vec_res2[i], vec_res2[i + 1]);
			(*out_s) << vec_res2[i] << "," << vec_res2[i + 1] << endl;
		}
	}
	else //string BASE32 BASE64
	{
		//vector<string> vec_res5 = querytest.RangeQueryByBruteforce_STR(rec, (SFCType)nsfc_type, (StringType)(nencode_type - 1));
		//print_ranges_str("hilbert 2d brute force", vec_res5);

		vector<string> vec_res6 = querytest.RangeQueryByRecursive_STR(rec, (SFCType)nsfc_type, (StringType)(nencode_type - 1), nranges);
		//print_ranges_str("hilbert 2d recursive", vec_res6);

		for (int i = 0; i < vec_res6.size(); i = i + 2)
		{
			//fprintf(output_file, "%s,%s\n", vec_res6[i].c_str(), vec_res6[i + 1].c_str());
			(*out_s) << vec_res6[i] << "," << vec_res6[i + 1] << endl;
		}
	}
	
	//if (output_file != NULL) fclose(output_file);
	range_file.close();

	tbb::tick_count t1 = tbb::tick_count::now();
	//cout << "time = " <<(t1 - t0).seconds()<<endl;

#endif


#ifdef RANDOM_LOD
	// random lod tests here
	RandomLOD<3> rnd_gen(10,20);

	int ncount[14] = { 0 };

	int nlevel=0;
	for (int i = 0; i < 10000000; i++)
	{
		nlevel = rnd_gen.RLOD_Gen();

		ncount[nlevel]++;
	}

	for (int i = 0; i < 14; i++)
	{
		cout << i << ":" << ncount[i] << endl;
	}

	cout << "out: " << rnd_gen.ntest << endl;
#endif
	//system("pause");
	return 0;
}

