#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include"Point.h"
#include<vector>

using std::vector;

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )

template< typename T, int nDims = 2>
class Rect
{
public:
	Point<T, nDims> minPoint;
	Point<T, nDims> maxPoint;
	int dimensions = nDims;
public:

	Rect(Point<T, nDims> minPoint, Point<T, nDims> maxPoint)
	{
		this->minPoint = minPoint;
		this->maxPoint = maxPoint;
		this->dimensions = nDims;
	}

	Rect()
	{

	}

	int GetDimensions()
	{
		return this->dimensions;
	}

	Point<T, nDims> GetMinPoint()
	{
		return this->minPoint;
	}

	Point<T, nDims> GetMaxPoint()
	{
		return this->maxPoint;
	}

	void SetMinPoint(Point<T, nDims> minpt)
	{
		this->minPoint = minpt;
	}

	void SetMaxPoint(Point<T, nDims> maxpt)
	{
		this->maxPoint = maxpt;
	}

	long GetDimWidth(int idx)
	{
		return this->maxPoint[idx] - this->minPoint[idx];
	}

	void DFS(vector<T> minVector, vector<T> maxVector,
		int i, vector<T > tmp, vector<vector<T >>& result)
	{
		if (i >= minVector.size() || i >= maxVector.size()) {
			if (!tmp.empty())
				result.push_back(tmp);
			return;
		}

		tmp.push_back(minVector[i]);
		DFS(minVector, maxVector, i + 1, tmp, result);
		tmp.pop_back();

		tmp.push_back(maxVector[i]);
		DFS(minVector, maxVector, i + 1, tmp, result);
		tmp.pop_back();
	}

	/*generate all the points of rectangle;
	for example ,from minPoint and the maxPoint,
	the 2d Rectangle will generate 4 points while the 3d Rectangle will
	generate 8 points.
	*/
	vector<Point<T, nDims>>
		GetAllCornerPts(Point<T, nDims> minPoint, Point<T, nDims> maxPoint)
	{
		vector<T>minVector;
		vector<T>maxVector;
		for (int i = 0; i < nDims; i++)
		{
			minVector.push_back(minPoint.getElements(i));
			maxVector.push_back(maxPoint.getElements(i));
		}
		vector<vector<T>> result;
		vector<T> tmp;
		DFS(minVector, maxVector, 0, vector<T>(), result);
		vector<Point<T, nDims>> Rec_Vector;
		Point<T, nDims> points;
		for (int i = 0; i < result.size(); ++i) {
			for (int j = 0; j < result[i].size(); ++j) {
				points[j] = result[i][j];
			}
			Rec_Vector.push_back(points);
		}
		return Rec_Vector;
	}
};


#endif