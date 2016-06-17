#ifndef QUERYBYSFC_H_
#define QUERYBYSFC_H_

//#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"
#include "OutputSchema.h"
#include "SFCConversion.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <queue>
#include <algorithm>

#include <time.h>

using namespace std;

#define RETURN_RANGES 40

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
	
	
	TreeNode()
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
	void query_approximate2(TreeNode<T, nDims> nd, Rect<T, nDims> queryrect, vector<TreeNode<T, nDims>>& resultTNode);
	int  iscontinuous(string& str1, string& str2);

public:
	vector<long long>  RangeQueryByBruteforce_LNG(Rect<T, nDims> queryRect, SFCType sfc_type);
	vector<long long>  RangeQueryByRecursive_LNG(Rect<T, nDims> queryrect, SFCType sfc_type);

	vector<string>  RangeQueryByBruteforce_STR(Rect<T, nDims> queryRect, SFCType sfc_type, StringType encode_type);
	vector<string>  RangeQueryByRecursive_STR(Rect<T, nDims> queryrect, SFCType sfc_type, StringType encode_type);
};


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

template<typename T, int nDims, int mBits>
void QueryBySFC<T, nDims, mBits>::query_approximate2(TreeNode<T, nDims> nd, Rect<T, nDims> queryrect, vector<TreeNode<T, nDims>>& resultTNode)
{
	int nary_num = 1 << nDims;  //max count: 2^nDims

	typedef tuple<TreeNode<T, nDims>, Rect<T, nDims>> NRTuple;
	queue<NRTuple> query_queue;

	TreeNode<T, nDims> nchild;
	int res, last_level;
	///////////////////////////////////////////
	//queue the root node
	query_queue.push(NRTuple(nd, queryrect));	
	last_level = 0;

	for (; !query_queue.empty(); query_queue.pop()) 
	{
		NRTuple currenttuple = query_queue.front();

		TreeNode<T, nDims> currentNode = std::get<0>(currenttuple);
		Rect<T, nDims> qrt = std::get<1>(currenttuple);

		//////////////////////////////////////////////////////
		//check the level and numbers of results
		if (last_level != currentNode.level && resultTNode.size() > RETURN_RANGES) //we are in the new level and full
		{
			break; //now
		}

		/////////////////////////////////////////////////////////////////////
		////get all children nodes till equal or intersect, if contain, continue to get children nodes
		do
		{
			for (int i = 0; i < nary_num; i++)
			{
				nchild = currentNode.GetChildNode(i);
				if (nchild.Spatialrelationship(qrt) == 0)  //equal: stop
				{
					resultTNode.push_back(nchild);
					res = 1;
					break; //break for and while ---to continue queue iteration
				}
				else if (nchild.Spatialrelationship(qrt) == 2)  //intersect: divide queryrectangle
				{
					res = 2;
					break;  //break for and while ---divide queryrectangle
				}
				else  if (nchild.Spatialrelationship(qrt) == 1)//contain: go down to the next level untill equal or intersect
				{
					res = 0;
					currentNode = nchild;
					break; //break for but to continue while
				}
			}//end for nary children
		} while (!res);

		if (res == 1) continue; //here break to continue for (queue iteration)
				
		//divide the input query rectangle into even parts, e.g. 2 or 4 parts
		//0~3 for 2d; upper 2|3----10|11;----- YX for 2D, ZYX for 3D, TZYX for 4D--each dim one bit
		//0~3 for 2d; lower 0|1----00|01 ------one dim: less = 0; greater = 1		
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
		qrtcut[0] = qrt;

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
			}//end for rect count

			ncount += newadd;  //update all rectangle count
		}//end for dimension

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
				//query_approximate(cNode, qrtcut[i], resultTNode);  //recursive query
				query_queue.push(NRTuple(cNode, qrtcut[i]));
			}		
		}//end for rect division check

	}///end for queue iteration
}

template< typename T, int nDims, int mBits>
vector<long long>  QueryBySFC<T, nDims, mBits>::RangeQueryByRecursive_LNG(Rect<T, nDims> queryrect, SFCType sfc_type)
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
		query_approximate2(root, queryrect, resultTNode);
	}

	vector<vector<Point<T, nDims>>> resultPoints;  //get all cell corner points
	for (int i = 0; i < resultTNode.size(); i++)
	{
		int ncount = 1;
		vector<Point<T, nDims>> nodePoints;  //cell points on i-th treeNode
		nodePoints.push_back(resultTNode[i].minPoint);
		for (int j = 0; j < nDims; j++)
		{
			int newadd = 0;
			for (int k = 0; k < ncount; k++) //1-->2;2-->4, --->2^Dims
			{
				Point<T, nDims> newPoint = nodePoints[k];
				newPoint[j] = resultTNode[i].maxPoint[j] - 1;  //get the cordinate from maxpoint in this dimension
				if (newPoint[j] != nodePoints[k][j])  //if newPoint equals to current points or not
				{
					nodePoints.push_back(newPoint);
					newadd++;
				}
			}
			ncount += newadd;
		}

		resultPoints.push_back(nodePoints);
	}

	vector<long long> result;

	long long val = 0;
	Point<long, mBits> pt;
	SFCConversion<nDims, mBits> sfc;
	OutputSchema<nDims, mBits> trans;

	for (int i = 0; i < resultPoints.size(); i++)
	{
		for (int j = 0; j < resultPoints[i].size(); j++)
		{
			sfc.ptCoord = resultPoints[i][j];
			if (sfc_type == Morton) sfc.MortonEncode();
			if (sfc_type == Hilbert) sfc.HilbertEncode();
			pt = sfc.ptBits;

			val = trans.BitSequence2Value(pt);
			result.push_back(val);
		}
	}

	int size = result.size();
	std::sort(result.begin(), result.end());

	vector<long long> rangevec;
	int nstart = 0;
	for (int i = 0; i < size - 1; i++)
	{
		if (result[i + 1] != (result[i] + 1))
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i]);

			nstart = i + 1;
		}

		if (result[i + 1] == (result[i] + 1) && (i + 1) == size - 1)
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i]);
		}		
	}
	
	return rangevec;
}

template< typename T, int nDims, int mBits>
vector<long long>  QueryBySFC<T, nDims, mBits>::RangeQueryByBruteforce_LNG(Rect<T, nDims> queryRect, SFCType sfc_type)
{
	Point<T, nDims> minPoint = queryRect.GetMinPoint();
	Point<T, nDims> maxPoint = queryRect.GetMaxPoint();

	long *difference = new long[nDims];
	long long *para = new long long[nDims + 1];

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
		for (long j = 0; j <= difference[i]; j++)
		{
			tempVector.push_back(temp + j);
		}
		queryVector.push_back(tempVector);
	}

	Point<T, nDims> point;

	long long tmp = para[nDims] - 1;
	SFCConversion<nDims, mBits> sfc;
	OutputSchema<nDims, mBits> trans;

	long long val = 0;
	long long  size = tmp + 1;
	long long* result = new long long[size];
	Point<long, mBits> pt;

	for (long long count = tmp; count >= 0; count--)
	{
		long long offset = count;
		for (int j = nDims - 1; j >= 0; j--)
		{
			long long div = para[j];
			int n = offset / div;
			offset = offset % div;
			point[j] = queryVector[j][n];
		}

		sfc.ptCoord = point;
		if (sfc_type == Morton) sfc.MortonEncode();
		if (sfc_type == Hilbert)  sfc.HilbertEncode();
		pt = sfc.ptBits;

		val = trans.BitSequence2Value(pt);
		result[count] = val;
	}

	delete[]para;
	delete[]difference;

	std::sort(result, result+size);

	vector<long long> rangevec;
	int nstart = 0;
	for (int i = 0; i < size - 1; i++)
	{
		if (result[i + 1] != (result[i] + 1))
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i]);

			nstart = i+1;
		}

		if (result[i + 1] == (result[i] + 1) && (i + 1) == size - 1)
		{
			rangevec.push_back(result[nstart]);
			rangevec.push_back(result[i]);
		}
	}


	delete[]result;
	return rangevec;
}

//////////////////////////////////////////////////////////////////////////////////////

template<typename T, int nDims, int mBits>
int QueryBySFC<T, nDims, mBits>::iscontinuous(string& str1, string& str2)
{
	if (str1.size() != str2.size()) return 0;
	int nsize = str1.size();

	if (str1.c_str()[nsize - 1] - str2.c_str()[nsize - 1] == 1) return 1;

	return 0;
}


template< typename T, int nDims, int mBits>
vector<string>  QueryBySFC<T, nDims, mBits>::RangeQueryByRecursive_STR(Rect<T, nDims> queryrect, SFCType sfc_type, StringType encode_type)
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
		query_approximate2(root, queryrect, resultTNode);
	}

	vector<vector<Point<T, nDims>>> resultPoints;  //get all cell corner points
	for (int i = 0; i < resultTNode.size(); i++)
	{
		int ncount = 1;
		vector<Point<T, nDims>> nodePoints;  //cell points on i-th treeNode
		nodePoints.push_back(resultTNode[i].minPoint);
		for (int j = 0; j < nDims; j++)
		{
			int newadd = 0;
			for (int k = 0; k < ncount; k++) //1-->2;2-->4, --->2^Dims
			{
				Point<T, nDims> newPoint = nodePoints[k];
				newPoint[j] = resultTNode[i].maxPoint[j] - 1;  //get the cordinate from maxpoint in this dimension
				if (newPoint[j] != nodePoints[k][j])  //if newPoint equals to current points or not
				{
					nodePoints.push_back(newPoint);
					newadd++;
				}
			}
			ncount += newadd;
		}

		resultPoints.push_back(nodePoints);
	}

	vector<string> result;
	string val;

	Point<long, mBits> pt;
	SFCConversion<nDims, mBits> sfc;
	OutputSchema<nDims, mBits> trans;

	for (int i = 0; i < resultPoints.size(); i++)
	{
		for (int j = 0; j < resultPoints[i].size(); j++)
		{
			sfc.ptCoord = resultPoints[i][j];
			if (sfc_type == Morton) sfc.MortonEncode();
			if (sfc_type == Hilbert) sfc.HilbertEncode();
			pt = sfc.ptBits;

			val = trans.BitSequence2String(pt, encode_type);
			result.push_back(val);
		}
	}

	int size = result.size();
	std::sort(result.begin(), result.end());

	vector<string> rangevec2;
	int nstart = 0;
	for (int i = 0; i < size - 1; i++)
	{
		if (!iscontinuous(result[i + 1], result[i]))
		{
			rangevec2.push_back(result[nstart]);
			rangevec2.push_back(result[i]);

			nstart = i + 1;
		}

		if (iscontinuous(result[i + 1], result[i]) && (i + 1) == size - 1)
		{
			rangevec2.push_back(result[nstart]);
			rangevec2.push_back(result[i]);
		}
	}

	return rangevec2;
}

template< typename T, int nDims, int mBits>
vector<string>  QueryBySFC<T, nDims, mBits>::RangeQueryByBruteforce_STR(Rect<T, nDims> queryRect, SFCType sfc_type, StringType encode_type)
{
	Point<T, nDims> minPoint = queryRect.GetMinPoint();
	Point<T, nDims> maxPoint = queryRect.GetMaxPoint();

	long *difference = new long[nDims];
	long long *para = new long long[nDims + 1];

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
		for (long j = 0; j <= difference[i]; j++)
		{
			tempVector.push_back(temp + j);
		}
		queryVector.push_back(tempVector);
	}

	Point<T, nDims> point;

	long long tmp = para[nDims] - 1;
	SFCConversion<nDims, mBits> sfc;
	OutputSchema<nDims, mBits> trans;
	Point<long, mBits> pt;
	
	long long size = tmp + 1;

	string val;
	vector<string> result;	

	for (long long count = tmp; count >= 0; count--)
	{ 
		//cout << count << endl;

		long long offset = count;
		for (int j = nDims - 1; j >= 0; j--)
		{
			long long div = para[j];
			long n = offset / div;
			offset = offset % div;
			point[j] = queryVector[j][n];
		}

		sfc.ptCoord = point;
		if (sfc_type == Morton) sfc.MortonEncode();
		if (sfc_type == Hilbert)  sfc.HilbertEncode();
		pt = sfc.ptBits;

		val = trans.BitSequence2String(pt, encode_type);
		result.push_back(val);
	}

	delete[]para;
	delete[]difference;

	std::sort(result.begin(), result.end());

	vector<string> rangevec2;
	int nstart = 0;
	for (int i = 0; i < size - 1; i++)
	{
		if (!iscontinuous(result[i + 1], result[i]))
		{
			rangevec2.push_back(result[nstart]);
			rangevec2.push_back(result[i]);

			nstart = i + 1;
		}

		if (iscontinuous(result[i + 1], result[i]) && (i + 1) == size - 1)
		{
			rangevec2.push_back(result[nstart]);
			rangevec2.push_back(result[i]);
		}
	}

	return rangevec2;
}

#endif