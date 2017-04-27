#ifndef COORDTRANSFORM_H_
#define COORDTRANSFORM_H_

#include "Point.h"

template<typename T1, typename T2, int nDims = 2>
class CoordTransform
{
private:
	std::array< T1, nDims > _delta;
	std::array< T1, nDims > _scale;

	/*Point<T1, nDims> inPt;
	Point<T2, nDims> outPt;*/

public:
	CoordTransform()
	{
		for (int i = 0; i < nDims; i++)
		{
			_delta[i] = 0;
			_scale[i] = 1;
		}

	}

	void SetTransform(T1* delta, T1* scale)
	{
		if (delta != NULL && scale != NULL)
		{
			for (int i = 0; i < nDims; i++)
			{
				_delta[i] = delta[i];
				_scale[i] = scale[i];
			}
		}
	}

	Point<T2, nDims> Transform(Point<T1, nDims> inPt)
	{
		Point<T2, nDims> outPt;
		for (int i = 0; i < nDims; i++)
		{
			/*double b = (inPt[i] - _delta[i])*_scale[i];
			outPt[i] = b ;
			double a = b - outPt[i];*/
			outPt[i] = lround((inPt[i] - _delta[i])*_scale[i]);
		}
		return outPt;
	}

	Point<T1, nDims> InverseTransform(Point<T2, nDims> inPt)
	{
		Point<T1, nDims> outPt;
		for (int i = 0; i < nDims; i++)
		{
			//outPt[i] = lround((inPt[i] - _delta[i])*_scale[i]);
			outPt[i] = ((double)inPt[i]) / _scale[i] + _delta[i]; //decoding
		}
		return outPt;
	}
};

#endif