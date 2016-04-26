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
#include "OutputSchema.h"

template<int nDims>
class InputItem
{
public:
	Point<double, nDims>* pPtsArray;
	int _actual_size;
	int _alloc_size;

	InputItem()
	{
		_alloc_size = 0;
		_actual_size = 0;

		pPtsArray = NULL;
	}
};

template<int nDims>
class OutputItem
{
public:
	Point<double, nDims>* pPtsArray;

	long* out_value;
	string* out_string;

	int _pt_alloc_size;
	int _actual_size;
	int _encode_mode;

	OutputItem()
	{
		_actual_size = 0;
		_encode_mode = 0;

		pPtsArray = NULL;
		out_value = NULL;
		out_string = NULL;
	}
};

template<int nDims>
class InputFilter : public tbb::filter
{
public:
	InputFilter(FILE* input_file_, int size) :
		filter(serial_in_order),
		input_file(input_file_),
		_size(size)
	{
	}

	~InputFilter()
	{}
private:
	FILE* input_file;
	int _size;
	/*override*/ void* operator()(void*)
	{
		// Read raw points coornidates
		InputItem<nDims>* pItem = (InputItem<nDims>*)tbb::tbb_allocator<InputItem<nDims>>().allocate(1);
		pItem->pPtsArray = (Point<double, nDims>*)tbb::tbb_allocator<Point<double, nDims>>().allocate(_size);
		pItem->_alloc_size = _size;

		char buf[1024];
		char * pch, * lastpos;
		char ele[64];

		int i, j ;
		i = 0;
		while (1) //always true
		{
			if (i == _size) break; //full, maximum _size;

			j = 0;
			memset(buf, 0, 1024);
			fgets(buf,1024, input_file);

			if (strlen(buf) == 0) break; // no more data

			lastpos = buf;
			pch = strchr(buf, ',');			
			while (pch != NULL)
			{
				memset(ele, 0, 64);
				strncpy_s(ele, 64, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				pItem->pPtsArray[i][j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			pItem->pPtsArray[i][j] = atof(lastpos); //final part

			i++;
		}

		pItem->_actual_size = i;

		if (i == 0)
		{
			tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)pItem->pPtsArray, pItem->_alloc_size);
			tbb::tbb_allocator<InputItem<nDims>>().deallocate((InputItem<nDims>*)pItem, 1);
			
			return NULL; //read nothing here, terminate
		}

		return pItem;
	}
};

//
/////////////////////////////////////////////////////
////Input coordinate transfomartion filter
//template<int nDims>
//class CoordTransFilter : public tbb::filter
//{
//public:
//	CoordTransFilter(int size);
//	/*override*/void* operator()(void* item);
//
//	void SetTransform(double* delta, long* scale);
//private:
//	int _size;
//	double* _delta;
//	long* _scale;
//};
//
//template<int nDims>
//CoordTransFilter::CoordTransFilter(int size) :
//tbb::filter(parallel),
//_size(size)
//{
//}
//
//template< typename T, int nDims>
//void CoordTransFilter::SetTransform(double* delta, long* scale)
//{
//	_delta = delta;
//	_scale = scale;
//}
//
//template<int nDims>
///*override*/void* CoordTransFilter::operator()(void* item)
//{
//	Point<double, nDims>*  input = static_cast<Point<double, nDims>* >(item);
//	Point<long, nDims>* output = (Point<long, nDims>*)tbb::tbb_allocator<Point<long, nDims>>().allocate(_size);
//	
//	CoordTransform<double, long, nDims> cotrans(_delta, _scale);
//	
//	for (int i = 0; i < _size; i++)
//	{
//		output[i] = cotrans.Transform(input[i]);
//	}
//
//	tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)input, _size);
//
//	return output;
//}
//
/////////////////////////////////////////////////////
////Input coordinate transfomartion filter
//template< int nDims, int mBits>
//class SFCGenFilter : public tbb::filter
//{
//public:
//	SFCGenFilter(int size, int sfctype);
//	/*override*/void* operator()(void* item);
//private:
//	int _size;
//	int _sfctype;
//};
//
//template< int nDims, int mBits>
//SFCGenFilter::SFCGenFilter(int size) :
//tbb::filter(parallel),
//_size(size),
//_sfctype(sfctype),
//{
//}
//
//template< int nDims, int mBits>
///*override*/void* SFCGenFilter::operator()(void* item)
//{
//	Point<long, nDims>*  input = static_cast<Point<long, nDims>*>(item);
//	Point<long, mBits>* output = (Point<long, mBits>*)tbb::tbb_allocator<Point<long, mBits>>().allocate(_size);
//
//	SFCConversion<nDims, mBits> sfcgen;
//	if (_sfctype == 0) //morton
//	{
//		for (int i = 0; i < _size; i++)
//		{
//			sfcgen.ptCoord = input[i];
//			sfcgen.MortonEncode();
//			output[i] = sfcgen.ptBits;
//		}
//	}
//	if (_sfctype == 1) //hilbert
//	{
//		for (int i = 0; i < _size; i++)
//		{
//			sfcgen.ptCoord = input[i];
//			sfcgen.HilbertEncode();
//			output[i] = sfcgen.ptBits;
//		}
//	}
//
//	tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)input, _size);
//
//	return output;
//
//}
//
/////////////////////////////////////////////////////
////Input coordinate transfomartion filter
//template< int nDims, int mBits>
//class BitsConvFilter : public tbb::filter
//{
//public:
//	BitsConvFilter(int size, int conv_type);
//	/*override*/void* operator()(void* item);
//private:
//	int _size;
//	int _conv_type;
//};
//
//template< int nDims, int mBits>
//BitsConvFilter::BitsConvFilter(int size) :
//tbb::filter(parallel),
//_size(size),
//_conv_type(conv_type)
//{
//}
//
//template< int nDims, int mBits>
///*override*/void* BitsConvFilter::operator()(void* item)
//{
//	Point<long, mBits>*  input = static_cast<Point<long, mBits>*>(item);
//	long* output = (long*)tbb::tbb_allocator<long>().allocate(_size);
//
//	OutputTransform<nDims, mBits> outtrans;
//	for (int i = 0; i < _size; i++)
//	{
//		output[i] = outtrans.BitSequence2Value(input[i]);
//	}
//
//
//	tbb::tbb_allocator<Point<long, mBits>>().deallocate((Point<long, mBits>*)input, _size);
//
//	return output;
//}

///////////////////////////////////////////////////
//new whole transfomartion filter
template<int nDims, int mBits>
class NewSFCGenFilter : public tbb::filter
{
public:
	NewSFCGenFilter(int sfctype, int conv_type, double* delta, long* scale) :
		tbb::filter(parallel),
		_sfctype(sfctype),
		_conv_type(conv_type)
	{
		if (delta != NULL && scale != NULL)
		{
			_delta = delta;
			_scale = scale;
		}
	}

	/*override*/void* operator()(void* item)
	{
		InputItem<nDims>*  pin_item = static_cast<InputItem<nDims>*>(item);
		Point<double, nDims>*  input = pin_item->pPtsArray;
		
		//////////////////////////////
		OutputItem<nDims>* pout_item = (OutputItem<nDims>*)tbb::tbb_allocator<OutputItem<nDims>>().allocate(1);
		pout_item->pPtsArray = pin_item->pPtsArray;
		pout_item->_actual_size = pin_item->_actual_size;
		pout_item->_pt_alloc_size = pin_item->_alloc_size;
		pout_item->_encode_mode = _conv_type;
		if (_conv_type == 0)
			pout_item->out_value = (long*)tbb::tbb_allocator<long>().allocate(pin_item->_actual_size);
		else
			pout_item->out_string = (string*)tbb::tbb_allocator<string>().allocate(pin_item->_actual_size);

		//////////////////////////////////////////////////////
		CoordTransform<double, long, nDims> cotrans;

		SFCConversion<nDims, mBits> sfcgen;
		OutputSchema<nDims, mBits> outtrans;

		if (_delta != NULL && _scale != NULL)
		{
			cotrans.SetTransform(_delta, _scale);
		}

		////Point<long, nDims> ptSFC;
		Point<long, mBits> ptBits;

		for (int i = 0; i < pin_item->_actual_size; i++)
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

			if (_conv_type == 0)
			{
				pout_item->out_value[i]= outtrans.BitSequence2Value(ptBits);
			}

			if (_conv_type == 1)
			{
				pout_item->out_string[i]= outtrans.BitSequence2String(ptBits, Base32);
			}

			if (_conv_type == 2)
			{
				pout_item->out_string[i] = outtrans.BitSequence2String(ptBits, Base64);
			}
		}

		////////////////
		tbb::tbb_allocator<InputItem<nDims>>().deallocate((InputItem<nDims>*)pin_item, 1); //only release the inputitem

		return pout_item;
	}

private:
	double* _delta;
	long* _scale;

	int _sfctype;
	int _conv_type;
};

//////////////////////////////////////////////////////////
//! Filter that writes each buffer to a file.
template<int nDims>
class OutputFilter : public tbb::filter
{
	FILE* output_file;
public:
	OutputFilter(FILE* output_file) :
		tbb::filter(serial_in_order),
		output_file(output_file)
	{
	}
	/*override*/void* operator()(void* item)
	{
		OutputItem<nDims>*  pout_item = static_cast<OutputItem<nDims>*>(item);

		///////////////////////
		for (int i = 0; i < pout_item->_actual_size; i++)
		{
			for (int j = 0; j < nDims; j++) 
			{
				//fwrite(input[i], sizeof(long), 1, my_output_file);
				fprintf_s(output_file, "%.6f", pout_item->pPtsArray[i][j]);
				fprintf_s(output_file, ",");
			}

			// one field for SFC code
			if (pout_item->_encode_mode == 0 ) ///value type
				fprintf_s(output_file, "%lu\n", pout_item->out_value[i]);
			else
				fprintf_s(output_file, "%s\n", pout_item->out_string[i].c_str());
		}

		///////////////////////
		tbb::tbb_allocator<Point<double, nDims>>().deallocate((Point<double, nDims>*)pout_item->pPtsArray, pout_item->_pt_alloc_size);
				
		if (pout_item->_encode_mode == 0)
			tbb::tbb_allocator<long>().deallocate((long*)pout_item->out_value, pout_item->_actual_size);
		else
			tbb::tbb_allocator<string>().deallocate((string*)pout_item->out_string, pout_item->_actual_size);

		tbb::tbb_allocator<OutputItem<nDims>>().deallocate((OutputItem<nDims>*)pout_item, 1);

		return NULL;
	}
};

template<int nDims, int mBits>
int run_pipeline(int nthreads, char* InputFileName, char* OutputFileName, \
	int item_num, int sfc_type, int conv_type, double* delta, long* scale)
{
	FILE* input_file = NULL;
	if (InputFileName != NULL && strlen(InputFileName) != 0)
	{
		fopen_s(&input_file, InputFileName, "r");
		if (!input_file)
		{
			return 0;
		}
	}
	else
	{
		input_file = stdin;
	}
	
	FILE* output_file = NULL;
	if (OutputFileName != NULL && strlen(OutputFileName) != 0)
	{
		fopen_s(&output_file, OutputFileName, "w");
		if (!output_file)
		{
			return 0;
		}
	}
	else
	{
		output_file = stdout;
	}	

	// Create the pipeline
	tbb::pipeline pipeline;

	// Create file-reading writing stage and add it to the pipeline
	InputFilter<nDims> input_filter(input_file, item_num);
	pipeline.add_filter(input_filter);

	// Create squaring stage and add it to the pipeline
	//CoordTransFilter<> coordtrans_filter;
	//pipeline.add_filter(coordtrans_filter);

	//SFCGenFilter<> sfcgen_filter;
	//pipeline.add_filter(sfcgen_filter);

	//BitsConvFilter<> bitsconv_filter;
	//pipeline.add_filter(bitsconv_filter);
	NewSFCGenFilter<nDims, mBits> nsfcgen_filter(sfc_type, conv_type, delta, scale);
	//nsfcgen_filter.SetTransform(delta, scale);
	pipeline.add_filter(nsfcgen_filter);

	// Create file-writing stage and add it to the pipeline
	OutputFilter<nDims> output_filter(output_file);
	pipeline.add_filter(output_filter);

	// Run the pipeline
	tbb::tick_count t0 = tbb::tick_count::now();
	// Need more than one token in flight per thread to keep all threads 
	// busy; 2-4 works
	pipeline.run(nthreads * 4);//
	tbb::tick_count t1 = tbb::tick_count::now();

	fclose(output_file);
	fclose(input_file);

	printf("time = %g\n", (t1 - t0).seconds());
	
	return 1;
}

#endif