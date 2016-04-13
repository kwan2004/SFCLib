#pragma once
#include "stdafx.h"
#include"Point.h"
#include"OutputTransform.h"
#include "SFCConversion.h"
#include<iostream>
#include <vector>
#include<stack>
using namespace std;

template< typename T, int nDims = 2, int mBits=4>
class QueryBySFC
{

private:
	void getResultInVector(vector<vector<T>> vec, int N, stack<T> tmp, vector<vector<T>>& tmp_result)
	{
		for (int i = 0; i< vec[N].size(); ++i)
		{
			tmp.push(vec.at(N).at(i));
			if (N<vec.size() - 1)
			{
				getResultInVector(vec, N + 1, tmp, tmp_result);
			}
			else
			{
				vector<T> one_result;
				vector<T> tmp_vector;
				int count = 0;
				while (!tmp.empty())
				{
					tmp_vector.push_back(tmp.top());
					tmp.pop();
					count++;
				}
				for (int i = 0; i<tmp_vector.size(); ++i)
				{

					//one_result.push_back(tmp.at(i));
					one_result.push_back(tmp_vector.at(count - i - 1));
					tmp.push(tmp_vector.at(count - i - 1));
				}
				tmp_result.push_back(one_result);
			}
			if (!tmp.empty())tmp.pop();
			//tmp.pop();
		}
	}

	vector<vector<T>> getAllCombination(vector<vector<T>>& vec)
	{
		stack<T> tmp_vec;
		vector<vector<T>> tmp_result;
		getResultInVector(vec, 0, tmp_vec, tmp_result);
		return tmp_result;
	}

public:

	QueryBySFC()
	{
		int i = 0;
	}

	void test()
	{
		vector<int> v1 = { 1, 2, 3 };
		vector<int> v2 = { 2, 3, 4 };
		vector<int> v3 = { 7, 8 };
		vector<vector<int>> input = { v1, v2, v3 };
		vector<vector<int>> result = getAllCombination(input);

	}

	vector<Point<T,nDims>> getAllPointsInQueryRec(Rectangle<T, nDims> queryRect)
	{
		Point<T, nDims> minPoint=queryRect.GetMinPoint();
		Point<T, nDims> maxPoint=queryRect.GetMaxPoint();
		vector<vector<T>> queryVector;
		for (int i = 0; i < nDims; i++)
		{
			vector<T> tempVector;
			int difference = maxPoint[i] - minPoint[i];
			T temp = minPoint[i];
			for(int j = 0; j <= difference; j++)
			{
				tempVector.push_back(temp + j);
			}
			queryVector.push_back(tempVector);
		}
		vector<vector<T>> result = getAllCombination(queryVector);
		vector<Point<T, nDims>> points;
		for (int i = 0; i < result.size(); ++i)
		{
			Point<T, nDims> tmp_point;
			for (int j = 0; j < result[i].size(); ++j)
			{
				tmp_point[j] = result[i][j];
			}
			points.push_back(tmp_point);
		}
		return points;
	}

	vector<long>  RangeQueryByMorton_Bruteforce(Rectangle<T, nDims> queryRect)
	{
		vector<Point<T, nDims>> points = getAllPointsInQueryRec(queryRect);
		vector<long> result;
		Point<long, mBits> pt;
		long val = 0;
		for (int i = 0; i < points.size(); i++)
		{
			SFCConversion<nDims, mBits> sfc;
			sfc.ptCoord = points[i];
			sfc.MortonEncode();
			OutputTransform<nDims, mBits> trans;
			pt = sfc.ptBits;
			val = trans.bitSequence2Value(pt);
			result.push_back(val);
			printf(" %d", val);
		}
		return result;
	}

};





