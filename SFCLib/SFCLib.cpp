// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"

#include "SFCConversion.h"
#include "QueryBySFC.h"
#include "OutputSchema.h"
#include "SFCPipeline.h"

#include "RandomLOD.h"

//#include "tbb/task_scheduler_init.h"

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

void print_ranges(char * str, vector<long long>& ranges)
{
	if (str == NULL) return;

	printf("%s \n", str);
	for (int i = 0; i < ranges.size(); i = i + 2)
	{
		//printf("\n");

		printf("%lld---%lld\n", ranges[i], ranges[i + 1]);

	}
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
	
	if (argc == 1) return 0;
	//if (argc % 2 != 1) return 0; //attribute pair plus exe_name

	const int ndims = 4;
	const int mbits = 30;

#ifdef PARALLEL_PIPELINE
	//-p 0 -s 1 -e 2 -t ct.txt -l 10 -i ahn2.txt -o ee.txt
	int nparallel = 0;

	int nsfc_type = 0;
	int nencode_type = 0;

	bool bisonlysfc = false;

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
			run_pipeline<ndims+1, mbits>(1, szinput, szoutput, 3000, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
		else
			run_pipeline<ndims, mbits>(1, szinput, szoutput, 3000, nsfc_type, nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);

	}

	if (nparallel == 1)
	{
		if (strlen(szoutput) != 0)  printf("parallel run "); //if not stdout ,print sth
		tbb::task_scheduler_init init_parallel;

		if (bislod)//lod value, one more dimension
			run_pipeline<ndims+1, mbits>(init_parallel.default_num_threads(), szinput, szoutput, 3000, nsfc_type, \
			nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
		else
			run_pipeline<ndims, mbits>(init_parallel.default_num_threads(), szinput, szoutput, 3000, nsfc_type, \
			nencode_type, delta, scale, bisonlysfc, bislod, lod_levels);
	}
	
#endif
	///////////////////////
#ifdef SFC_GEN_QUERY
	Point<double, 4> pt1;
	pt1[0] = 85224.3;//1010
	pt1[1] = 447071.86;//1011
	pt1[2] = 0.39; //0011
	pt1[3] = 9;//1101

	Point<double, 4> pt2;
	//pt2[0] = 85224.3;//1010
	//pt2[1] = 447071.86;//1011
	//pt2[2] = 0.4; //0011
	//pt2[3] = 9;//1101
	pt2[0] = 85098.38;//1010
	pt2[1] = 446440.06;//1011
	pt2[2] = 18.34; //0011
	pt2[3] = 9;//1101

	double delta[4] = { 80000.00, 437500.00, -20.0, 0.0 }; // 526000, 4333000, 300
	long  scale[4] = { 100, 100, 100, 1 }; //100, 100, 1000

	CoordTransform<double, long, ndims> cotrans;
	cotrans.SetTransform(delta, scale);

	Point<long, 4> MinPt2 = cotrans.Transform(pt1);
	Point<long, 4> MaxPt2 = cotrans.Transform(pt2);

	SFCConversion<4, 30> sfctest;
	OutputSchema<4, 30> transtest;

	sfctest.ptCoord = MinPt2;
	//sfc.MortonEncode();
	sfctest.HilbertEncode();
	Point<long, 30> ptbts1; 
	ptbts1 = sfctest.ptBits;
	cout << transtest.BitSequence2String(ptbts1, Base64).c_str() << endl;

	sfctest.ptCoord = MaxPt2;
	//sfc.MortonEncode();
	sfctest.HilbertEncode();
	Point<long, 30> ptbts2;
	ptbts2 = sfctest.ptBits;
	cout << transtest.BitSequence2String(ptbts2, Base64).c_str() << endl;

	string res("+++++++MZcZE4Sxf+BdL");//85098.38 446440.06 18.34 9

	SFCConversion<4, 30> sfctest2;
	OutputSchema<4, 30> transtest2;

	Point<long, 30> ptbts3 = transtest2.String2BitSequence(res, Base64);
	sfctest2.ptBits = ptbts3;
	sfctest2.HilbertDecode();
	Point<long, 4> Pt3 = sfctest2.ptCoord;

	

	///////////////////////////////////////
	////2D case 8*8, i.e n=2, m=3
	Point<long, 2> ptCoord; //SFC coordinates n=2
	Point<long, 3> ptBits; //SFC bit sequence m=3

	SFCConversion<2, 3> sfc;
	OutputSchema<2, 3> trans;
		
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ptCoord[0] = i;//i
			ptCoord[1] = j;//j

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
			
			long outval = trans.BitSequence2Value(ptBits);
			string strout = trans.BitSequence2String(ptBits, Base64);

			cout << i << ", " << j << "====" << outval << " , " << strout << endl;
		}
	}

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
	long Point1[2] = { 3, 2};
	long Point2[2] = { 5, 5 };
	Point<long, 2> MinPoint(Point1);
	Point<long, 2> MaxPoint(Point2);
	Rect<long, 2> rec(MinPoint, MaxPoint);
	QueryBySFC<long, 2, 3> querytest;
	vector<long long> vec_res = querytest.RangeQueryByBruteforce_LNG(rec, Hilbert);
	print_ranges("hilbert 2d brute force", vec_res);

	vector<long long> vec_res2 = querytest.RangeQueryByRecursive_LNG(rec, Hilbert,0);
	print_ranges("hilbert 2d recursive", vec_res2);

	vector<string> vec_res5 = querytest.RangeQueryByBruteforce_STR(rec, Hilbert, Base64);
	print_ranges_str("hilbert 2d brute force", vec_res5);

	vector<string> vec_res6 = querytest.RangeQueryByRecursive_STR(rec, Hilbert, Base64,0);
	print_ranges_str("hilbert 2d recursive", vec_res6);


	////3D sample
	long Point31[3] = { 4, 2, 5 };
	long Point32[3] = { 5, 4, 7 };
	Point<long, 3> MinPoint3(Point31);
	Point<long, 3> MaxPoint3(Point32);

	Rect<long, 3> rec3(MinPoint3, MaxPoint3);

	QueryBySFC<long, 3, 10> querytest3;
	vector<long long> vec_res3 = querytest3.RangeQueryByBruteforce_LNG(rec3, Morton);
	print_ranges("morton 3d brute force", vec_res3);

	vector<long long> vec_res4 = querytest3.RangeQueryByRecursive_LNG(rec3, Morton,0);
	print_ranges("morton 3d recursive", vec_res4);

	vector<string> vec_res7 = querytest3.RangeQueryByBruteforce_STR(rec3, Hilbert, Base64);
	print_ranges_str("hilbert 2d brute force", vec_res7);

	vector<string> vec_res8 = querytest3.RangeQueryByRecursive_STR(rec3, Hilbert, Base64,0);
	print_ranges_str("hilbert 2d recursive", vec_res8);

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
	//-i 85098.0/85099.0/446444.0/446445.0/12/13/6/9 -s 1 -e 2 -t ct.txt -n 10000 -o qq.sql
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
	
	CoordTransform<double, long, ndims> cotrans;
	cotrans.SetTransform(delta, scale);
	////////////////////////////////////////////////
	//get the input filter
	double pt1[ndims] = { 0.0f };
	double pt2[ndims] = { 0.0f };

	memset(pt1, 0, sizeof(double)*ndims);
	memset(pt2, 0, sizeof(double)*ndims);

	char * pch, *lastpos;
	char ele[64];

	lastpos = szinput;
	for (int i = 0; i < ndims; i++)
	{
		///////min
		memset(ele, 0, 64);
		
		pch = strchr(lastpos, '//');
		strncpy(ele, lastpos, pch - lastpos);
		pt1[i] = atof(ele);

		lastpos = pch + 1;
		///////max
		if (i != ndims - 1)
		{
			memset(ele, 0, 64);

			pch = strchr(lastpos, '//');
			strncpy(ele, lastpos, pch - lastpos);
			pt2[i] = atof(ele);

			lastpos = pch + 1;
		}
		else
		{
			pt2[i] = atof(lastpos);
		}

	}
	///////////////////////////////////////////////
	//point transfomration
	Point<double, ndims> MinPt1(pt1);
	Point<double, ndims> MaxPt1(pt2);

	Point<long, ndims> MinPt2 = cotrans.Transform(MinPt1);
	Point<long, ndims> MaxPt2 = cotrans.Transform(MaxPt1);

	/////////////////////////////////////////////////////
	////query
	Rect<long, ndims> rec(MinPt2, MaxPt2);
	QueryBySFC<long , ndims, mbits> querytest;

	FILE* output_file = NULL;
	if (szoutput != NULL && strlen(szoutput) != 0)
	{
		output_file = fopen(szoutput, "w");
		if (!output_file)
		{
			return 0;
		}
	}
	else
	{
		output_file = stdout;
	}

	if (nencode_type == 0) //number
	{
		//vector<long long> vec_res = querytest.RangeQueryByBruteforce_LNG(rec, (SFCType)nsfc_type);
		//print_ranges("hilbert 2d brute force", vec_res);

		vector<long long> vec_res2 = querytest.RangeQueryByRecursive_LNG(rec, (SFCType)nsfc_type, nranges);
		//print_ranges("hilbert 2d recursive", vec_res2);
		for (int i = 0; i < vec_res2.size(); i = i + 2)
		{
			fprintf(output_file, "%lld,%lld\n", vec_res2[i], vec_res2[i + 1]);
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
			fprintf(output_file, "%s,%s\n", vec_res6[i].c_str(), vec_res6[i + 1].c_str());
		}
	}
	
	if (output_file != NULL) fclose(output_file);
#endif


#ifdef RANDOM_LOD

	RandomLOD<3> rnd_gen(14,20);

	int ncount[14] = { 0 };

	int nlevel=0;
	for (int i = 0; i < 1000000; i++)
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

