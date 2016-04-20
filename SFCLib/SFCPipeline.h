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

class InputFilter : public tbb::filter 
{
public:
	InputFilter(FILE* input_file_);
	~InputFilter();
private:
	FILE* input_file;
	/*override*/ void* operator()(void*);
};

InputFilter::InputFilter(FILE* input_file_) :
filter(serial_in_order),
input_file(input_file_)
{
}

InputFilter::~InputFilter()
{

}

void* InputFilter::operator()(void*)
{
	// Read characters into space that is available in the next slice.

}

///////////////////////////////////////////////////
//Input coordinate transfomartion filter
class CoordTransformFilter : public tbb::filter 
{
public:
	CoordTransformFilter();
	/*override*/void* operator()(void* item);
};

CoordTransformFilter::CoordTransformFilter() :
tbb::filter(parallel)
{
}

/*override*/void* CoordTransformFilter::operator()(void* item)
{
	
}

///////////////////////////////////////////////////
//Input coordinate transfomartion filter
class SFCGenFilter : public tbb::filter
{
public:
	SFCGenFilter();
	/*override*/void* operator()(void* item);
};

SFCGenFilter::SFCGenFilter() :
tbb::filter(parallel)
{
}

/*override*/void* SFCGenFilter::operator()(void* item)
{

}
///////////////////////////////////////////////////
//Input coordinate transfomartion filter
class BitsConvFilter : public tbb::filter
{
public:
	BitsConvFilter();
	/*override*/void* operator()(void* item);
};

BitsConvFilter::BitsConvFilter() :
tbb::filter(parallel)
{
}

/*override*/void* BitsConvFilter::operator()(void* item)
{

}

//! Filter that writes each buffer to a file.
class OutputFilter : public tbb::filter 
{
	FILE* my_output_file;
public:
	OutputFilter(FILE* output_file);
	/*override*/void* operator()(void* item);
};

OutputFilter::OutputFilter(FILE* output_file) :
tbb::filter(serial_in_order),
my_output_file(output_file)
{
}

void* OutputFilter::operator()(void* item)
{

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
	InputFilter input_filter(input_file);
	pipeline.add_filter(input_filter);

	// Create squaring stage and add it to the pipeline
	CoordTransformFilter coordtrans_filter;
	pipeline.add_filter(coordtrans_filter);

	SFCGenFilter sfcgen_filter;
	pipeline.add_filter(sfcgen_filter);

	BitsConvFilter bitsconv_filter;
	pipeline.add_filter(bitsconv_filter);

	// Create file-writing stage and add it to the pipeline
	OutputFilter output_filter(output_file);
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