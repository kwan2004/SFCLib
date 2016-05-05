#ifndef QUERYBYSFC_H_
#define QUERYBYSFC_H_

#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"
#include "OutputSchema.h"
#include "SFCConversion.h"
#include <iostream>
#include <vector>

#include <time.h>

using namespace std;

typedef enum
{
	Morton,
	Hilbert,
} SFCType;

template<typename T, int nDims = 2>
class TreeNode
{
public:
	int level;  //which level: i-th level
	Point<T, nDims> minPoint;
	Point<T, nDims> maxPoint;
	int dimensions = nDims;

	void operator=(TreeNode const& other)
	{
		level = other.level;
		minPoint = other.minPoint;
		maxPoint = other.maxPoint;
		dimensions = other.dimensions;
	}
	
	TreeNode<T, nDims>::TreeNode()
	{
	}

	/*
	return the idx-th childnode
	one dim, less than middle is 0, bigger than middle is 1
	0~3 for 2d; upper 2|3----10|11;-----So: YX for 2D, ZYX for 3D, TZYX for 4D
	            lower 0|1----00|01 ---------put next dimension before current dimension
	*/
	TreeNode<T, nDims> GetChildNode(int idx)
	{
		TreeNode<T, nDims> nchild;
		nchild.minPoint = this->minPoint;
		nchild.maxPoint = this->maxPoint;

		for (int i = 0; i < nDims; i++)
		{
			if ((idx >> i) & 1)  //the bit on the i-th dimension is 1: bigger
			{
				nchild.minPoint[i] = (this->minPoint[i] + this->maxPoint[i]) / 2;
			}
			else  //the bit on the i-th dimension is 0: smaller
			{
				nchild.maxPoint[i] = (this->minPoint[i] + this->maxPoint[i]) / 2;
			}
		}

		nchild.level = this->level + 1;

		return nchild;
	}

	/*
	return the relationship between treenode and queryRectangle
	0: treenode is equal to queryRectangle
	1: treenode contains queryRectangle
	2: treenode intersects queryRectangle
	-1(default): not overlap
	*/
	int Spatialrelationship(Rect<T, nDims> qrt)
	{
		/*
		equal: 
		if (nrt.x0 == qrt.x0 && nrt.y0 == qrt.y0 &&
		nrt.x1 == qrt.x1 && nrt.y1 == qrt.y1)
		return 0;
		*/
		int ncmp = 1;
		for (int i = 0; i < nDims; i++)
		{
			ncmp &= this->minPoint[i] == qrt.minPoint[i] && this->maxPoint[i] == qrt.maxPoint[i];
		}
		if (ncmp) return 0;

		/*
		fully contain:
		if (nrt.x0 <= qrt.x0 && nrt.y0 <= qrt.y0 &&
		nrt.x1 >= qrt.x1 && nrt.y1 >= qrt.y1)
		return 1;
		*/
		ncmp = 1;
		for (int i = 0; i < nDims; i++)
		{
			ncmp &= this->minPoint[i] <= qrt.minPoint[i] && this->maxPoint[i] >= qrt.maxPoint[i];
		}
		if (ncmp) return 1;

		/*
		intersect:
		//http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
		RectA.Left < RectB.Right && RectA.Right > RectB.Left && RectA.Top > RectB.Bottom && RectA.Bottom < RectB.Top
		this can be extended more dimensions
		//http://stackoverflow.com/questions/5009526/overlapping-cubes
		if (nrt.x0 < qrt.x1 && nrt.x1 > qrt.x0 &&
		nrt.y0 < qrt.y1 && nrt.y1 > qrt.y0)
		return 2;
		*/
		ncmp = 1;
		for (int i = 0; i < nDims; i++)
		{
			ncmp &= this->minPoint[i] < qrt.maxPoint[i] && this->maxPoint[i] > qrt.minPoint[i];
		}
		if (ncmp) return 2;

		//not overlap
		return -1;
	}
};

template<typename T, int nDims = 2, int mBits = 4>
class QueryBySFC
{
private:
	//vector<Point<T, nDims>> getAllPoints(Rect<T, nDims> queryRect);
	
	void query_approximate(TreeNode<T, nDims> nd, Rect<T, nDims> queryrect, vector<TreeNode<T, nDims>>& resultTNode);

public:
	vector<long>  RangeQueryByBruteforce(Rect<T, nDims> queryRect, SFCType code_type);
	vector<long>  RangeQueryByRecursive(Rect<T, nDims> queryrect, SFCType code_type);

};

/*
template<typename T, int nDims, int mBits>
vector<Point<T, nDims>> QueryBySFC<T, nDims, mBits>::getAllPoints(Rect<T, nDims> queryRect)
{
	Point<T, nDims> minPoint = queryRect.GetMinPoint();
	Point<T, nDims> maxPoint = queryRect.GetMaxPoint();

	long *difference = new long[nDims];
	long *para = new long[nDims + 1];

	para[0] = 1;
	for (int i = 0; i < nDims; i++)
	{
		difference[i] = maxPoint[i] - minPoint[i] + 1;
		para[i + 1] = para[i] * difference[i];
	}


	vector<vector<T>> queryVector;
	for (int i = 0; i < nDims; i++)
	{
		vector<T> tempVector;
		//int difference = maxPoint[i] - minPoint[i];
		T temp = minPoint[i];
		for (int j = 0; j <= difference[i]; j++)
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
*/

template<typename T, int nDims, int mBits>
void QueryBySFC<T, nDims, mBits>::query_approximate(TreeNode<T, nDims> nd, Rect<T, nDims> queryrect, vector<TreeNode<T, nDims>>& resultTNode)
{
	/*
	divide current tree node
	*/
	int nary_num = 1 << nDims;  //max count: 2^nDims
	vector<TreeNode<T, nDims>> nchild(nary_num);
	/*
	find the currentnode exactly contains queryrectangle; and its child node intersects queryrectangle
	*/
	TreeNode<T, nDims> currentNode = nd;
	int res = 1;
	do
	{
		for (int i = 0; i < nary_num; i++)
		{
			nchild[i] = currentNode.GetChildNode(i);
			if (nchild[i].Spatialrelationship(queryrect) == 0)  //equal: stop
			{
				resultTNode.push_back(nchild[i]);
				return ;
			}
			else if (nchild[i].Spatialrelationship(queryrect) == 2)  //intersect: divide queryrectangle
			{
				res = 0;
				break;
			}
			else  if (nchild[i].Spatialrelationship(queryrect) == 1)//contain: divide the tree node
			{
				currentNode = nchild[i];
				break;
			}
		}
	} while (res);

	
	/*
	divide the input query rectangle into even parts, e.g. 2 or 4 parts
	0~3 for 2d; upper 2|3----10|11;----- YX for 2D, ZYX for 3D, TZYX for 4D--each dim one bit
	            lower 0|1----00|01 ------one dim: less = 0; greater = 1
	*/
	
	vector<Rect<T, nDims>> qrtcut(nary_num);  //2^nDims parts
	vector<int> qrtpos(nary_num);  //the qrtcut corresponds to treenode
	for (int i = 0; i < nary_num; i++)
	{
		qrtpos[i] = 0;
	}
	vector<int> mid(nDims);  //middle cut line--dim number
	for (int i = 0; i < nDims; i++)
	{
		mid[i] = (currentNode.minPoint[i] + currentNode.maxPoint[i]) / 2;
	}

	int ncount = 1;
	qrtcut[0] = queryrect;
	
	Point<T, nDims> pttmp;  //temporary point or corner
	for (int i = 0; i < nDims; i++)  //dimension iteration
	{
		int newadd = 0;
		for (int j = 0; j < ncount; j++)
		{
			if (qrtcut[j].minPoint[i] < mid[i] && qrtcut[j].maxPoint[i] > mid[i])
			{
				Rect<T, nDims> rtnew = qrtcut[j];
				pttmp = rtnew.minPoint;
				pttmp[i] = mid[i];
				rtnew.SetMinPoint(pttmp);

				pttmp = qrtcut[j].maxPoint;
				pttmp[i] = mid[i];
				qrtcut[j].SetMaxPoint(pttmp);

				qrtpos[ncount + newadd] = (1 << i) + qrtpos[j];
				qrtcut[ncount + newadd] = rtnew;

				newadd++;
			}

			if (qrtcut[j].minPoint[i] >= mid[i])  //all bigger than the middle line
			{
				qrtpos[j] |= 1 << i;  //just update its position---put 1 on the dimension bit
			}
		}  //end for rect count

		ncount += newadd;  //update all rectangle count
	}  //end for dimension
	
	for (int i = 0; i < ncount; i++)   //final rect number 
	{
		TreeNode<T, nDims> cNode = currentNode.GetChildNode(qrtpos[i]);
		int rec = cNode.Spatialrelationship(qrtcut[i]);
		if (rec == 0)
		{
			resultTNode.push_back(cNode);  //equal
		}
		else if (rec == -1)
		{
		}
		else
		{
			query_approximate(cNode, qrtcut[i], resultTNode);  //recursive query
		}		
	}
}

template< typename T, int nDims, int mBits>
vector<long>  QueryBySFC<T, nDims, mBits>::RangeQueryByRecursive(Rect<T, nDims> queryrect, SFCType code_type)
{
	vector<TreeNode<T, nDims>> resultTNode;  //tree nodes correspond to queryRectangle
	TreeNode<T, nDims> root;  //root node
	root.level = 0;
	for (int i = 0; i < nDims; i++)
	{
		root.minPoint[i] = 0;
		root.maxPoint[i] = 1 << mBits;
		queryrect.maxPoint[i] += 1;
	}

	int res = root.Spatialrelationship(queryrect);
	if (res == 0)  //equal
	{
		resultTNode.push_back(root);
	}
	if (res == 1)  //contain
	{
		query_approximate(root, queryrect, resultTNode);
	}

	vector<vector<Point<T, nDims>>> resultPoints;  //all cell points
	for (int i = 0; i < resultTNode.size(); i++)
	{
		int ncount = 1;
		vector<Point<T, nDims>> nodePoints;  //cell points on i-th treeNode
		nodePoints.push_back(resultTNode[i].minPoint);
		for (int j = 0; j < nDims; j++)
		{
			int newadd = 0;
			for (int k = 0; k < ncount; k++)
			{
				Point<T, nDims> newPoint = nodePoints[k];
				newPoint[j] = resultTNode[i].maxPoint[j] - 1;  //cell point = maxPoint - 1
				if (newPoint[j] != nodePoints[k][j])  //if newPoint equals to current points or not
				{
					nodePoints.push_back(newPoint);
					newadd++;
				}
			}
			ncount += newadd;
		}

		resultPoints.push_back(nodePoints);

		//cout << "level: " << resultTNode[i].level << "\t" << endl;
	}

	vector<vector<long>> resultCode;
	long val = 0;
	Point<long, mBits> pt;
	if (code_type == Morton)
	{
		for (int i = 0; i < resultPoints.size(); i++)
		{
			vector<long> temCode;
			for (int j = 0; j < resultPoints[i].size(); j++)
			{
				for (int k = 0; k < nDims; k++)
				{
					cout << resultPoints[i][j][k] << "\t";
				}

				SFCConversion<nDims, mBits> sfc;
				sfc.ptCoord = resultPoints[i][j];
				//sfc.HilbertEncode();
				sfc.MortonEncode();
				OutputSchema<nDims, mBits> trans;
				pt = sfc.ptBits;
				val = trans.BitSequence2Value(pt);
				temCode.push_back(val);

				cout << val << endl;
			}

			resultCode.push_back(temCode);
		}
	}
	else if (code_type == Hilbert)
	{
		for (int i = 0; i < resultPoints.size(); i++)
		{
			vector<long> temCode;
			for (int j = 0; j < resultPoints[i].size(); j++)
			{
				for (int k = 0; k < nDims; k++)
				{
					cout << resultPoints[i][j][k] << "\t";
				}

				SFCConversion<nDims, mBits> sfc;
				sfc.ptCoord = resultPoints[i][j];
				sfc.HilbertEncode();
				//sfc.MortonEncode();
				OutputSchema<nDims, mBits> trans;
				pt = sfc.ptBits;
				val = trans.BitSequence2Value(pt);
				temCode.push_back(val);

				cout << val << endl;
			}

			resultCode.push_back(temCode);
		}
	}

	vector<long> vec_return;

	//vector<vector<long>> results(resultCode.size());
	long min, max;
	for (int i = 0; i < resultCode.size(); i++)
	{
		min = resultCode[i][0];
		max = resultCode[i][0];
		for (int j = 0; j < resultCode[i].size(); j++)
		{
			if (resultCode[i][j] < min)
			{
				min = resultCode[i][j];
			}
			if (resultCode[i][j] > max)
			{
				max = resultCode[i][j];
			}
		}
		if (min != max)
		{
			//results[i].push_back(min);
			//results[i].push_back(max);
			vec_return.push_back(min);
			vec_return.push_back(max);
		}
		else
		{
			//results[i].push_back(min);
			vec_return.push_back(min);
			vec_return.push_back(min);
		}
	}

	/*for (int i = 0; i < vec_return.size(); i=i+2)
	{
		cout << vec_return[i] << "\t" << vec_return[i+1] << endl;
	}*/

	return vec_return;
}


template< typename T, int nDims, int mBits>
vector<long>  QueryBySFC<T, nDims, mBits>::RangeQueryByBruteforce(Rect<T, nDims> queryRect, SFCType code_type)
{
	Point<T, nDims> minPoint = queryRect.GetMinPoint();
	Point<T, nDims> maxPoint = queryRect.GetMaxPoint();

	long *difference = new long[nDims];
	long *para = new long[nDims + 1];

	para[0] = 1;
	for (int i = 0; i < nDims; i++)
	{
		difference[i] = maxPoint[i] - minPoint[i] + 1;// for brute force , needs to add 1
		para[i + 1] = para[i] * difference[i]; //the coordinates are in the cell center
	}

	vector<vector<T>> queryVector;
	for (int i = 0; i < nDims; i++)
	{
		vector<T> tempVector;
		//int difference = maxPoint[i] - minPoint[i];
		T temp = minPoint[i];
		for (int j = 0; j <= difference[i]; j++)
		{
			tempVector.push_back(temp + j);
		}
		queryVector.push_back(tempVector);
	}

	Point<T, nDims> point;
	//vector <Point<T, nDims>> points;
	long tmp = para[nDims] - 1;
	SFCConversion<nDims, mBits> sfc;
	OutputSchema<nDims, mBits> trans;
	long val = 0;
	int size = tmp + 1;
	long* result = new long[size];
	Point<long, mBits> pt;
	if (code_type == Morton)
	{
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
			sfc.ptCoord = point;
			sfc.MortonEncode();
			pt = sfc.ptBits;
			val = trans.BitSequence2Value(pt);
			result[count] = val;
			//points.push_back(point);
		}
	}
	else if (code_type == Hilbert)
	{
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
			sfc.ptCoord = point;
			sfc.HilbertEncode();
			pt = sfc.ptBits;
			val = trans.BitSequence2Value(pt);
			result[count] = val;
			//points.push_back(point);
		}
	}	

	delete[]para;
	delete[]difference;

	//sort the morton values
	std::sort(result, result + size);

	///the test code.it can be deleted
	//////////////////////////////////////
	/*printf("\n morton sort result: \n");
	for (int i = 0; i < size; i++)
	{
		printf("%d\t", result[i]);
	}
	printf("\n");*/

	vector<long> rangevec;
	int nstart = 0;
	for (int i = 0; i < size - 1; i++)
	{
		if (result[i + 1] != (result[i] + 1))
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i]);

			//printf("%d---%d\n", result[nstart], result[i]);

			nstart = i+1;
		}

		if (result[i + 1] == (result[i] + 1) && (i + 1) == size - 1)
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i]);

			//printf("%d---%d\n", result[nstart], result[i]);
		}
		/*if (result[i + 1] == (result[i] + 1))
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
		}*/
	}

	//printf("\n morton final result: \n");
	//for (int i = 0; i < rangevec.size(); i = i + 2)
	//{
	//	//printf("\n");
	//	
	//	printf("%d---%d\n", rangevec[i], rangevec[i + 1]);

	//}
	//printf("\n");

	delete[]result;
	return rangevec;
}
#endif