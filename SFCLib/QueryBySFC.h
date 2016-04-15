#pragma once
#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"
#include "OutputTransform.h"
#include "SFCConversion.h"

#include <iostream>
#include <vector>

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
			difference[i] = maxPoint[i] - minPoint[i] + 1;
		}
		long *para = new long[nDims + 1];
		para[0] = 1;
		for (int i = 1; i <= nDims; i++)
		{
			long tmp = difference[i - 1];
			para[i] = para[i - 1] * tmp;

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
		delete[]para;
		delete[]difference;
		return result;
	}

	int query_approximate(TreeNode nd, Rectangle<T, nDims> qrect);

public:

	QueryBySFC()
	{

	}


	vector<vector<long>>  RangeQueryByMorton_Bruteforce(Rectangle<T, nDims> queryRect)
	{
		//get all the points in the query range
		vector<Point<T, nDims>> points = getAllPoints(queryRect);
		int size = points.size();
		long* result = new long[size];
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
			result[i] = val;
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

		delete[]result;
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

		delete[]result;
		return resultVector;
	}

	RangeQueryByRecursive(Rectangle<T, nDims> queryrect);
};

template< typename T, int nDims = 2>
class TreeNode : public Rectangle<T, nDims>
{
public:
	//int ndim;
	int level;

	//one dim, less than middle is 0, bigger than middle is 1
	//0~3 for 2d; upper 2|3----10|11;-----So: YX for 2D, ZYX for 3D, TZYX for 4D
	//            lower 0|1----00|01 ---------put next dimension before current dimension

	TreeNode GetChildNode(int idx)
	{
		TreeNode nchild(this);

		for (int i = 0; i < nDims; i++)
		{
			if ((idx >> i) & 1) // bit on each ith dimension 1: bigger
			{
				nchild.minPoint[i] = (this->minPoint[i] + this->maxPoint[i])/2;
			}
			else  //0 smaller in this dimension
			{
				nchild.maxPoint[i] = (this->minPoint[i] + this->maxPoint[i]) / 2;
			}
		}
		
		nchild.level = this->level + 1;

		return nchild;
	}

	int Spatialrelationship(Rectangle<T, nDims> qrt)//0 = equal; 1=contain;2=intersect; default = -1  not overlap ;
	{
		//rect nrt(this->x0, this->y0, this->x0 + this->width, this->y0 + this->width);
		///equal
		/*if (nrt.x0 == qrt.x0 && nrt.y0 == qrt.y0 && \
			nrt.x1 == qrt.x1 && nrt.y1 == qrt.y1)
			return 0;*/
		int ncmp = 1;
		for (int i = 0; i < nDims; i++)
		{
			ncmp &= this->minPoint[i] == qrt.GetMinPoint()[i] && this->maxPoint[i] == qrt.GetMaxPoint()[i];
		}
		if (ncmp) return 0;

		//fully contain(how about edge touch?)
		/*if (nrt.x0 <= qrt.x0 && nrt.y0 <= qrt.y0 && \
			nrt.x1 >= qrt.x1 && nrt.y1 >= qrt.y1)
			return 1*/
		ncmp = 1;
		for (int i = 0; i < nDims; i++)
		{
			ncmp &= this->minPoint[i] <= qrt.GetMinPoint()[i] && this->maxPoint[i] >= qrt.GetMaxPoint()[i];
		}
		if (ncmp) return 1;

		//intersect
		///http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
		///RectA.Left < RectB.Right && RectA.Right > RectB.Left && RectA.Top > RectB.Bottom && RectA.Bottom < RectB.Top
		// this can be extended more dimensions
		//http://stackoverflow.com/questions/5009526/overlapping-cubes
		/*if (nrt.x0 < qrt.x1 && nrt.x1 > qrt.x0 && \
			nrt.y0 < qrt.y1 && nrt.y1 > qrt.y0)
			return 2;*/
		ncmp = 1;
		for (int i = 0; i < nDims; i++)
		{
			ncmp &= this->minPoint[i] < qrt.GetMaxPoint()[i] && this->maxPoint[i] > qrt.GetMinPoint()[i];
		}
		if (ncmp) return 2;

		return -1; //not overlap
	}
};

/////////////////////////////////////////////////////////////
///comparison between the tree node and the query rectangle
template< typename T, int nDims, int mBits>
int QueryBySFC::query_approximate(TreeNode nd, Rectangle<T, nDims> queryrect)
{
	////this tree node now fully contains the input query rectangle
	////firstly to check this node is the leaf node: yes, stop here
	if (nd.level == mBits)
	{
		/*printf("node level : %d, width %d, orig (%d, %d), dest (%d, %d)\n", \
			nd.level, nd.width, nd.x0, nd.y0, nd.x0 + nd.width, nd.y0 + nd.width);

		treenode2sfcinterval(nd);*/
		return 0;
	}

	////check the spatial relationship between this query rectangle and four children
	TreeNode nchild[2 ^ nDims];

	for (int i = 0; i < 2 ^ nDims; i++)
	{
		nchild[i] = nd.GetChildNode(i);
	}

	for (int i = 0; i < 2 ^ nDims; i++)
	{
		int nrt = nchild[i].Spatialrelationship(qrt);

		if (nrt == 0)//equal to one child, that's enough, stop here
		{
			/*printf("node level : %d, width %d, orig (%d, %d), dest (%d, %d)\n", \
				nchild[i].level, nchild[i].width, nchild[i].x0, nchild[i].y0, \
				nchild[i].x0 + nchild[i].width, nchild[i].y0 + nchild[i].width);*/

			//treenode2sfcinterval(nchild[i]);
			return 0;
		}
		if (nrt == 1)//fully contained by one child,then go down directly
		{
			query_approximate(nchild[i], qrt);
			return 0;
		}

		//intersect one child (i.e. smaller than this node and bigger than one child), this means further division
		if (nrt == 2)
		{
			break;
		}
	}

	/////this tree node is divided to get the 4 child nodes
	/////also to divide the input query rectangle into 2 or 4 parts(ONLY 2 or 4 here!!!)
	//0~3 for 2d; upper 2|3----10|11;----- YX for 2D, ZYX for 3D, TZYX for 4D--each dim one bit
	//            lower 0|1----00|01 ------one dim: less = 0; greater = 1

	array<Rectangle<T, nDims>, 2 ^ nDims> rtcut; //maximum results 2*dim
	int rtpos[2 ^ nDims] = { 0 };

	int mid[nDims]; // middle cut line--dim number
	for (int i = 0; i < nDims; i++)
	{
		mid[i] = ( nd.GetMinPoint()[i] + nd.GetMaxPoint()[i] )/ 2;
	}

	int ncount = 1;
	rtcut[0] = qrt;

	Point<T, nDims> pttmp;
	for (int i = 0; i < nDims; i++) //dimension iteration
	{
		int newadd = 0;
		for (int j = 0; j < ncount; j++)
		{
			//0 is less, 1 is bigger
			
			if (rtcut[j].GetMinPoint()[i] < mid[i] && rtcut[j].GetMaxPoint()[i] > mid[i]) //true in the middle of this dimension
			{
				/////add one rectangle, the new rect is in the bigger area
				rect rtnew = rtcut[j];

				//cut this rectangle along the middle line
				pttmp = rtnew.GetMinPoint();//bigger
				pttmp[i] = mid[i]; 
				rtnew.SetMinPoint(pttmp);

				pttmp = rtcut.GetMaxPoint();//smaller
				pttmp[i] = mid[i];
				rtcut.SetMaxPoint(pttmp);

				rtpos[ncount + newadd] = (1 << i) + rtpos[j]; //--put 1 on the dimension bit
				rtcut[ncount + newadd] = rtnew;

				newadd++;
			}

			if (rtcut[j].GetMinPoint()[i] >= mid[i]) //all bigger than the middle line
			{
				rtpos[j] |= 1 << i; //just update its position---put 1 on the dimension bit
			}
		}//end for rect count

		ncount += newadd; //update all rectangle count
	}//end for dimension

	for (int j = 0; j < ncount; j++) //final rect number 
	{
		query_approximate(nchild[rtpos[j]], rtcut[j]);
	}

	return 0;
}


template< typename T, int nDims, int mBits>
QueryBySFC::RangeQueryByRecursive(Rectangle<T, nDims> queryrect)
{
	TreeNode<T, nDims> root;
	root.level = 0;


	//rect qrt;
	//qrt.x0 = 3; qrt.x1 = 6; //here,point coordidate is located on lef-bottom of the correspoind cell
	//qrt.y0 = 2; qrt.y1 = 6; //so the right-top coordidate equals cell center + 1

	/*qrt.x0 = 2; qrt.x1 = 5;
	qrt.y0 = 1; qrt.y1 = 3;*/

	///check if the root node contains or eqauls the query rectangle
	int res = root.Spatialrelationship(queryrect);
	if (res == 0) ///equals
	{
		;
		/*printf("node level : %d, width %d, orig (%d, %d), dest (%d, %d)\n", \
		root.level, root.width, root.x0, root.y0, root.x0 + root.width, root.y0 + root.width);*/
	}

	if (res == 1) //fully contain 
	{
		query_approximate(root, qrt);
	}
}




