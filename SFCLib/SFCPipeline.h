#ifndef SFCPIPELINE_H_
#define SFCPIPELINE_H_

#include "tbb/pipeline.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tbb_allocator.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include "common/utility/utility.h"

#include "Point.h"
#include "CoordTransform.h"
#include "SFCConversion.h"
#include "OutputTransform.h"

template< typename T, int nDims>
class InputFilter : public tbb::filter 
{
public:
	InputFilter(FILE* input_file_, int size);
	~InputFilter();
private:
	FILE* input_file;
	int _size;
	/*override*/ void* operator()(void*);
};

template< typename T, int nDims>
InputFilter::InputFilter(FILE* input_file_, int size) :
filter(serial_in_order),
input_file(input_file_),
_size(size)
{
}

template< typename T, int nDims>
InputFilter::~InputFilter()
{

}

template< typename T, int nDims>
void* InputFilter::operator()(void*)
{
	// Read raw points coornidates
	Point<double, nDims>* pPtsArray = (Point<double, nDims>*)tbb::tbb_allocator<Point<double, nDims>>().allocate(_size);
	for (int i = 0; i < _size; i++)
	{
		for (int j = 0; j < nDims; j++)
		{
			pPtsArray[i][j] = 0;
		}
	}

	return pPtsArray;
}

///////////////////////////////////////////////////
//Input coordinate transfomartion filter
template< typename T, int nDims>
class CoordTransFilter : public tbb::filter
{
public:
	CoordTransFilter(int size);
	/*override*/void* operator()(void* item);

	void SetTransform(double* delta, long* scale);
private:
	int _size;
	double* _delta;
	long* _scale;
};

template< typename T, int nDims>
CoordTransFilter::CoordTransFilter(int size) :
tbb::filter(parallel),
_size(size)
{
}

template< typename T, int nDims>
void CoordTransFilter::SetTransform(double* delta, long* scale)
{
	_delta = delta;
	_scale = scale;
}

template< typename T, int nDims>
/*override*/void* CoordTransFilter::operator()(void* item)
{
	Point<double, nDims>*  input = static_cast<Point<double, nDims>* >(item);
	Point<long, nDims>* output = (Point<long, nDims>*)tbb::tbb_allocator<Point<long, nDims>>().allocate(_size);
	
	CoordTransform<double, long, nDims> cotrans(_delta, _scale);
	
	for (int i = 0; i < _size; i++)
	{
		output[i] = cotrans.Transform(input[i]);
	}

	tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)input, _size);

	return output;
}

///////////////////////////////////////////////////
//Input coordinate transfomartion filter
template< typename T, int nDims, int mBits>
class SFCGenFilter : public tbb::filter
{
public:
	SFCGenFilter(int size, int sfctype);
	/*override*/void* operator()(void* item);
private:
	int _size;
	int _sfctype;
};

template< typename T, int nDims, int mBits>
SFCGenFilter::SFCGenFilter(int size) :
tbb::filter(parallel),
_size(size),
_sfctype(sfctype),
{
}

template< typename T, int nDims, int mBits>
/*override*/void* SFCGenFilter::operator()(void* item)
{
	Point<long, nDims>*  input = static_cast<Point<long, nDims>*>(item);
	Point<long, mBits>* output = (Point<long, mBits>*)tbb::tbb_allocator<Point<long, mBits>>().allocate(_size);

	CSFCConversion<nDims, mBits> sfcgen;
	if (_sfctype == 0) //morton
	{
		for (int i = 0; i < _size; i++)
		{
			sfcgen.ptCoord = input[i];
			sfcgen.MortonEncode();
			output[i] = sfcgen.ptBits;
		}
	}
	if (_sfctype == 1) //hilbert
	{
		for (int i = 0; i < _size; i++)
		{
			sfcgen.ptCoord = input[i];
			sfcgen.HilbertEncode();
			output[i] = sfcgen.ptBits;
		}
	}

	tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)input, _size);

	return output;

}

///////////////////////////////////////////////////
//Input coordinate transfomartion filter
template< typename T, int nDims, int mBits>
class BitsConvFilter : public tbb::filter
{
public:
	BitsConvFilter(int size, int conv_type);
	/*override*/void* operator()(void* item);
private:
	int _size;
	int _conv_type;
};

template< typename T, int nDims, int mBits>
BitsConvFilter::BitsConvFilter(int size) :
tbb::filter(parallel),
_size(size),
_conv_type(conv_type)
{
}

template< typename T, int nDims, int mBits>
/*override*/void* BitsConvFilter::operator()(void* item)
{
	Point<long, mBits>*  input = static_cast<Point<long, mBits>*>(item);
	long* output = (long*)tbb::tbb_allocator<long>().allocate(_size);

	OutputTransform<nDims, mBits> outtrans;
	for (int i = 0; i < _size; i++)
	{
		output[i] = outtrans.BitSequence2Value(input[i]);
	}


	tbb::tbb_allocator<Point<long, mBits>>().deallocate((Point<long, mBits>*)input, _size);

	return output;
}

///////////////////////////////////////////////////
//new whole transfomartion filter
template< typename T, int nDims, int mBits>
class NewSFCGenFilter : public tbb::filter
{
public:
	NewSFCGenFilter(int size, int sfctype);
	/*override*/void* operator()(void* item);

	void SetTransform(double* delta, long* scale);
private:
	int _size;

	double* _delta;
	long* _scale;

	int _sfctype;

	int _conv_type;
};

template< typename T, int nDims, int mBits>
NewSFCGenFilter::NewSFCGenFilter(int size, int sfctype) :
tbb::filter(parallel),
_size(size),
_sfctype(sfctype),
{
}

template< typename T, int nDims, int mBits>
/*override*/void* NewSFCGenFilter::operator()(void* item)
{
	Point<double, nDims>*  input = static_cast<Point<double, nDims>* >(item);
	long* output = (long*)tbb::tbb_allocator<long>().allocate(_size);

	CoordTransform<double, long, nDims> cotrans(_delta, _scale);
	CSFCConversion<nDims, mBits> sfcgen;
	OutputTransform<nDims, mBits> outtrans;

	//Point<long, nDims> ptSFC;
	Point<long, mBits> ptBits;

	for (int i = 0; i < _size; i++)
	{
		//ptSFC = cotrans.Transform(input[i]);
		if (_sfctype == 0) //morton
		{
			sfcgen.ptCoord = cotrans.Transform(input[i]);
			sfcgen.MortonEncode();
			ptBits = sfcgen.ptBits;
		}
		
		if (_sfctype == 1) //hilbert
		{
			sfcgen.ptCoord = cotrans.Transform(input[i]);
			sfcgen.HilbertEncode();
			ptBits = sfcgen.ptBits;
		}

		output[i] = outtrans.BitSequence2Value(ptBits);
	}

	tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)input, _size);

	return output;
}

//////////////////////////////////////////////////////////
//! Filter that writes each buffer to a file.
template<int nDims>
class OutputFilter : public tbb::filter 
{
	FILE* my_output_file;
public:
	OutputFilter(FILE* output_file, int size);
	/*override*/void* operator()(void* item);
private:
	int _size;
};

template<int nDims>
OutputFilter::OutputFilter(FILE* output_file, int size) :
tbb::filter(serial_in_order),
my_output_file(output_file),
_size(size)
{
}

template<int nDims>
void* OutputFilter::operator()(void* item)
{
	long*  input = static_cast<long* >(item);

	for (int i = 0; i < _size; i++)
	{
		for (int j = 0; j < nDims; i++)
		{
			fwrite(input[i], sizeof(long), 1, my_output_file);
		}
	}

	return NULL;
}

bool silent = false;

int run_pipeline(int nthreads)
{
	FILE* input_file = fopen(InputFileName.c_str(), "r");
	if (!input_file) 
	{
		throw std::invalid_argument(("Invalid input file name: " + InputFileName).c_str());
		return 0;
	}
	FILE* output_file = fopen(OutputFileName.c_str(), "w");
	if (!output_file) 
	{
		throw std::invalid_argument(("Invalid output file name: " + OutputFileName).c_str());
		return 0;
	}

	// Create the pipeline
	tbb::pipeline pipeline;

	// Create file-reading writing stage and add it to the pipeline
	InputFilter<> input_filter(input_file);
	pipeline.add_filter(input_filter);

	// Create squaring stage and add it to the pipeline
	CoordTransFilter<> coordtrans_filter;
	pipeline.add_filter(coordtrans_filter);

	SFCGenFilter<> sfcgen_filter;
	pipeline.add_filter(sfcgen_filter);

	BitsConvFilter<> bitsconv_filter;
	pipeline.add_filter(bitsconv_filter);

	// Create file-writing stage and add it to the pipeline
	OutputFilter<> output_filter(output_file);
	pipeline.add_filter(output_filter);

	// Run the pipeline
	tbb::tick_count t0 = tbb::tick_count::now();
	// Need more than one token in flight per thread to keep all threads 
	// busy; 2-4 works
	pipeline.run(nthreads * 4);
	tbb::tick_count t1 = tbb::tick_count::now();

	fclose(output_file);
	fclose(input_file);

	if (!silent) 
		printf("time = %g\n", (t1 - t0).seconds());

	return 1;
}

#endif