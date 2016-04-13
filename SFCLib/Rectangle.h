#pragma once
#ifndef RECTANGLE_H_
#define RECTANGLE_H_
#include "Point.h"
#include <vector>

using std::vector;

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )
template< typename T, int nDimensions=2>
class Rectangle
{
public:
	Point<T,nDimensions> minPoint;
	Point<T,nDimensions> maxPoint;
	int dimensions = nDimensions;
public:

	Rectangle(Point<T, nDimensions> minPoint, Point<T,nDimensions> maxPoint)
	{
		this->minPoint = minPoint;
		this->maxPoint = maxPoint;
		this->dimensions = nDimensions;
	}

	int GetDimensions()
	{
		return this->dimensions;
	}

	Point<T, nDimensions> GetMinPoint()
	{
		return this->minPoint;
	}

	Point<T, nDimensions> GetMaxPoint()
	{
		return this->maxPoint;
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
	vector<Point<T, nDimensions>>
    GetAllCornerPts(Point<T, nDimensions> minPoint, Point<T, nDimensions> maxPoint)
	{
		vector<T>minVector;
		vector<T>maxVector;
		for (int i = 0; i < nDimensions; i++)
		{
			minVector.push_back(minPoint.getElements(i));
			maxVector.push_back(maxPoint.getElements(i));
		}
		vector<vector<T>> result;
		vector<T> tmp;
		DFS(minVector, maxVector, 0, vector<T>(), result);
		vector<Point<T, nDimensions>> Rec_Vector;
		Point<T, nDimensions> points;
		for (int i = 0; i < result.size(); ++i) {
			for (int j = 0; j < result[i].size(); ++j) {
				points[j]=result[i][j];
			}
			   Rec_Vector.push_back(points);
		}
		return Rec_Vector;
	}
};


#endif