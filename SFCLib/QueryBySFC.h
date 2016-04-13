#pragma once
#include "stdafx.h"

#include "Point.h"
#include "OutputTransform.h"
#include "SFCConversion.h"

#include <iostream>
#include <vector>
#include <stack>

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

	vector<vector<long>>  RangeQueryByMorton_Bruteforce(Rectangle<T, nDims> queryRect)
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
		}
		std::sort(result.begin(),result.end());

		/////the test code.it can be deleted
		////////////////////////////////////////
		printf("\n morton sort result: \n");
		for (int i = 0; i < result.size(); i++)
		{
			printf("%d\t", result[i]);
		}
		printf("\n");
		/////////////////////////////////////////

		long tmp = result[0];
		vector<long> tmpVector;
		tmpVector.push_back(tmp);
		vector<vector<long>> resultVector;
		for (int i = 0; i < result.size()-1; i++)
		{
			if (result[i+1] == (result[i]+1))
			{
				tmpVector.push_back(result[i + 1]);
				if ((i + 1) == result.size() - 1)
				{
					vector<long> eachRange;
					eachRange.push_back(tmpVector[0]);
					eachRange.push_back(result[i + 1]);
					resultVector.push_back(eachRange);
				}
			}
			if (result[i + 1] != (result[i] + 1))
			{
				vector<long> eachRange;
				if (tmpVector.size()>1)
				{
					eachRange.push_back(tmpVector[0]);
					eachRange.push_back(tmpVector[tmpVector.size()-1]);
				}
				else
				{
					eachRange.push_back(tmpVector[0]);
				}
				resultVector.push_back(eachRange);
				tmpVector.clear();
				tmpVector.push_back(result[i + 1]);
				if ((i+1) == result.size()-1)
				{
					resultVector.push_back(tmpVector);
				}

			}
		}

		/////the test code.it can be deleted
	    //////////////////////////////////////////
		printf("\n morton final result: \n");
		for (int i = 0; i < resultVector.size(); i++)
		{
			printf("\n");
			for (int j = 0; j < resultVector[i].size(); j++)
			{
				printf("%d\t", resultVector[i][j]);
			}

		}
		printf("\n");
		///////////////////////////////////////////////
		return resultVector;
	}

	vector<vector<long>>  RangeQueryByHilbert_Bruteforce(Rectangle<T, nDims> queryRect)
	{
		vector<Point<T, nDims>> points = getAllPointsInQueryRec(queryRect);
		vector<long> result;
		Point<long, mBits> pt;
		long val = 0;

		for (int i = 0; i < points.size(); i++)
		{
			SFCConversion<nDims, mBits> sfc;
			sfc.ptCoord = points[i];
			sfc.HilbertEncode();
			OutputTransform<nDims, mBits> trans;
			pt = sfc.ptBits;
			val = trans.bitSequence2Value(pt);
			result.push_back(val);
		}
		std::sort(result.begin(), result.end());

		/////the test code.it can be deleted
		////////////////////////////////////////
		printf("\n hilbert sort result: \n");
		for (int i = 0; i < result.size(); i++)
		{
			printf("%d\t", result[i]);
		}
		printf("\n");
		/////////////////////////////////////////

		long tmp = result[0];
		vector<long> tmpVector;
		tmpVector.push_back(tmp);
		vector<vector<long>> resultVector;
		for (int i = 0; i < result.size() - 1; i++)
		{
			if (result[i + 1] == (result[i] + 1))
			{
				tmpVector.push_back(result[i + 1]);
				if ((i + 1) == result.size() - 1)
				{
					vector<long> eachRange;
					eachRange.push_back(tmpVector[0]);
					eachRange.push_back(result[i + 1]);
					resultVector.push_back(eachRange);
				}
			}
			if (result[i + 1] != (result[i] + 1))
			{
				vector<long> eachRange;
				if (tmpVector.size()>1)
				{
					eachRange.push_back(tmpVector[0]);
					eachRange.push_back(tmpVector[tmpVector.size() - 1]);
				}
				else
				{
					eachRange.push_back(tmpVector[0]);
				}
				resultVector.push_back(eachRange);
				tmpVector.clear();
				tmpVector.push_back(result[i + 1]);
				if ((i + 1) == result.size() - 1)
				{
					resultVector.push_back(tmpVector);
				}

			}
		}

		/////the test code.it can be deleted
		//////////////////////////////////////////
		printf("\n hilbert final result: \n");
		for (int i = 0; i < resultVector.size(); i++)
		{
			printf("\n");
			for (int j = 0; j < resultVector[i].size(); j++)
			{
				printf("%d\t", resultVector[i][j]);
			}

		}
		printf("\n");
		///////////////////////////////////////////////
		return resultVector;
	}

};





