#pragma once
#include "stdafx.h"
#include"Point.h"
#include"OutputTransform.h"
#include "SFCConversion.h"
#include<iostream>
#include <vector>
#include<stack>
#include<time.h>

using namespace std;

template< typename T, int nDims = 2, int mBits=4>
class QueryBySFC
{

private:



	vector<Point<T, nDims>> getAllPoints(Rectangle<T, nDims> queryRect)
	{
		Point<T, nDims> minPoint = queryRect.GetMinPoint();
		Point<T, nDims> maxPoint = queryRect.GetMaxPoint();
		long *difference = new long[nDims];
		for (int i = 0; i < nDims; i++)
		{
			difference[i]= maxPoint[i]-minPoint[i]+1;
		}
		long *para = new long[nDims+1];
		para[0] = 1;
		for (int i = 1; i <= nDims; i++)
		{
			long tmp = difference[i - 1];
			para[i] = para[i - 1]*tmp;

		}
		vector<vector<T>> queryVector;
		for (int i = 0; i < nDims; i++)
		{
			vector<T> tempVector;
			int difference = maxPoint[i] - minPoint[i];
			T temp = minPoint[i];
			for (int j = 0; j <= difference; j++)
			{
				tempVector.push_back(temp + j);
			}
			queryVector.push_back(tempVector);
		}

		Point<T, nDims> point;
		vector <Point<T, nDims>> result;
		long tmp = para[nDims] - 1;
		for (int count = tmp; count >= 0; count--)
		{
			long offset = count;
			for (int j = nDims - 1; j >= 0; j--)
			{
				long div = para[j];
				int n = offset / div;
				offset = offset % div;
				point[j] = queryVector[j][n];
			}
			result.push_back(point);
		}

		delete []para;
		delete []difference;

		return result;
	}


public:

	QueryBySFC()
	{

	}


	vector<vector<long>>  RangeQueryByMorton_Bruteforce(Rectangle<T, nDims> queryRect)
	{
		//get all the points in the query range
		vector<Point<T, nDims>> points = getAllPoints(queryRect);
		int size = points.size();
		long* result =new long[size];
		Point<long, mBits> pt;
		long val = 0;

		//get all the morton codes in  the query range from the points just generated
		for (int i = 0; i <size; i++)
		{
			SFCConversion<nDims, mBits> sfc;
			sfc.ptCoord = points[i];
			sfc.MortonEncode();
			OutputTransform<nDims, mBits> trans;
			pt = sfc.ptBits;
			val = trans.bitSequence2Value(pt);
			result[i]=val;
		}

		//sort the morton values
		std::sort(result, result + size);
		///the test code.it can be deleted
		//////////////////////////////////////
		printf("\n morton sort result: \n");
		for (int i = 0; i < size; i++)
		{
			printf("%d\t", result[i]);
		}
		printf("\n");
		///////////////////////////////////////

		// get the final result 
		vector<vector<long>> resultVector;
		int flag = 0;
		for (int i = 0; i < size-1; i++)
		{
			if (result[i+1] == (result[i]+1))
			{
				if ((i + 1) == size - 1)
				{
					vector<long> eachRange;
					eachRange.push_back(result[flag]);
					eachRange.push_back(result[i + 1]);
					resultVector.push_back(eachRange);
				}
				continue;
			}
			if (result[i + 1] != (result[i] + 1))
			{
				vector<long> eachRange;
				if (i-flag>0)
				{
					eachRange.push_back(result[flag]);
					eachRange.push_back(result[i]);
				}
				else
				{
					eachRange.push_back(result[flag]);
				}
				resultVector.push_back(eachRange);
				flag = i+1;
				if (i + 1== size - 1)
				{
					vector<long>  last = { result[flag] };
					resultVector.push_back(last);
				}
			}
		}
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
		return resultVector;
	}

	vector<vector<long>>  RangeQueryByHilbert_Bruteforce(Rectangle<T, nDims> queryRect)
	{

		//get all the points in the query range
		vector<Point<T, nDims>> points = getAllPoints(queryRect);
		int size = points.size();
		long* result = new long[size];
		Point<long, mBits> pt;
		long val = 0;

		//get all the hilbert codes in  the query range from the points just generated
		for (int i = 0; i <size; i++)
		{
			SFCConversion<nDims, mBits> sfc;
			sfc.ptCoord = points[i];
			sfc.HilbertEncode();
			OutputTransform<nDims, mBits> trans;
			pt = sfc.ptBits;
			val = trans.bitSequence2Value(pt);
			result[i] = val;
		}

		//sort the hilbert values
		std::sort(result, result + size);

		///the test code.it can be deleted
		//////////////////////////////////////
		printf("\n hilbert sort result: \n");
		for (int i = 0; i < size; i++)
		{
			printf("%d\t", result[i]);
		}
		printf("\n");
		///////////////////////////////////////

        //get the final result 
		vector<vector<long>> resultVector;
		int flag = 0;
		for (int i = 0; i < size - 1; i++)
		{
			if (result[i + 1] == (result[i] + 1))
			{
				if ((i + 1) == size - 1)
				{
					vector<long> eachRange;
					eachRange.push_back(result[flag]);
					eachRange.push_back(result[i + 1]);
					resultVector.push_back(eachRange);
				}
				continue;
			}
			if (result[i + 1] != (result[i] + 1))
			{
				vector<long> eachRange;
				if (i - flag>0)
				{
					eachRange.push_back(result[flag]);
					eachRange.push_back(result[i]);
				}
				else
				{
					eachRange.push_back(result[flag]);
				}
				resultVector.push_back(eachRange);
				flag = i + 1;
				if (i + 1 == size - 1)
				{
					vector<long>  last = { result[flag] };
					resultVector.push_back(last);
				}
			}
		}


		///the test code.it can be deleted
		//////////////////////////////////////
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
		//////////////////////////////////////

		return resultVector;
	}


	



};





