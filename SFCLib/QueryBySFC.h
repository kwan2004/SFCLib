#ifndef QUERYBYSFC_H_
#define QUERYBYSFC_H_

#include <stdlib.h>
#include "Point.h"
#include "Rectangle.h"

#include "SFCConversion.h"

#include <vector>
using namespace std;

template< typename T, int nDims>
class node
{
public:
	//int ndim;

	T x0;
	T y0;

	int width;
	int level;

	//one dim, less than middle is 0, bigger than middle is 1
	//0~3 for 2d; upper 2|3----10|11;-----S0: YX for 2D, ZYX for 3D, TZYX for 4D
	//            lower 0|1----00|01

	node Getchildnode(int idx)
	{
		node nchild;

		//if (idx == 0 || idx == 2)
		//	nchild.x0 = this->x0;
		//else
		//	nchild.x0 = this->x0 + this->width / 2;

		//if (idx == 0 || idx == 1)
		//	nchild.y0 = this->y0;
		//else
		//	nchild.y0 = this->y0 + this->width / 2;

		nchild.width = this->width / 2;
		nchild.level = this->level + 1;

		return nchild;
	}

	int SpatialRelationship(Rectangle<T, nDims> queryrect)//0 = equal; 1=contain;2=intersect; default = -1  not overlap ;
	{
		rect nrt(this->x0, this->y0, this->x0 + this->width, this->y0 + this->width);
		///equal
		if (nrt.x0 == qrt.x0 && nrt.y0 == qrt.y0 && \
			nrt.x1 == qrt.x1 && nrt.y1 == qrt.y1)
			return 0;

		//fully contain(how about edge touch?)
		if (nrt.x0 <= qrt.x0 && nrt.y0 <= qrt.y0 && \
			nrt.x1 >= qrt.x1 && nrt.y1 >= qrt.y1)
			return 1;

		//intersect
		///http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
		///RectA.Left < RectB.Right && RectA.Right > RectB.Left && RectA.Top > RectB.Bottom && RectA.Bottom < RectB.Top
		// this can be extended more dimensions
		//http://stackoverflow.com/questions/5009526/overlapping-cubes
		if (nrt.x0 < qrt.x1 && nrt.x1 > qrt.x0 && \
			nrt.y1 > qrt.y0 && nrt.y0 < qrt.y1)
			return 2;

		return -1; //not overlap
	}
};

template< typename T, int nDims, int mBits>
class SFCQuery
{
public:
	RangeQueryByBruteforce(Rectangle<T, nDims> queryrect);
	RangeQueryByRecursive(Rectangle<T, nDims> queryrect);

private:
	int query_approximate(node nd, Rectangle<T, nDims> qrect);

};

template< typename T, int nDims, int mBits>
SFCQuery::RangeQueryByBruteforce(Rectangle<T, nDims> queryrect)
{
	/*int x0, x1;
	int y0, y1;

	x0 = 3; x1 = 5;
	y0 = 2; y1 = 5;

	x0 = 2; x1 = 4;
	y0 = 1; y1 = 2;

	int rows = x1 - x0 + 1;
	int cols = y1 - y0 + 1;

	int* pcodelist = new int(rows*cols);
	memset(pcodelist, 0, sizeof(int)*rows*cols);*/
	long totalcells = 1;
	std::array< long, nDims> dimwidth;
	for (int i = 0; i < nDims; i++)
	{
		dimwidth[i] = queryrect.GetDimWidth(i) + 1;
		totalcells *= dimwidth[i];
	}

	//#pragma omp for schedule(dynamic)
	std::array< long, nDims> dimpos;
	Point<long, nDims> pt;
	Point<long, mBits> pt2;

	SFCConversion<nDims, mBits> sfc;
	OutputTransform<nDims, mBits> trans;

	for (int i = 0; i < totalcells; i++)
	{
		for (int j = 0; j < nDims; j++)
		{
			pt[j] = dimpos[j];
		}
		////////////
		
		sfc2.ptCoord = pt;
		sfc2.HilbertEncode();
		pt2 = sfc2.ptBits;

		long val = trans2.bitSequence2Value(pt2);

		printf("query cell: %d, %d, %d ---   %d \n", row, col, (row - x0)*cols + (col - y0), nidx2);
	}


	/*for (int i = 0; i < rows*cols; i++)
	{
		printf("before sort: %d, %d \n", i, pcodelist[i]);
	}*/


	qsort(pcodelist, rows*cols, sizeof(int), compare);

	//for (int i = 0; i < rows*cols; i++)
	//{
	//	printf("after sort: %d, %d \n", i, pcodelist[i]);
	//}

	int nstart = 0;
	int ncur = nstart;
	while (ncur < cols*rows)
	{
		ncur++;
		if (pcodelist[ncur] - pcodelist[ncur - 1] != 1) // not continuous
		{
			printf_s("%d, %d\n", pcodelist[nstart], pcodelist[ncur - 1]);

			nstart = ncur;
		}
	}
}


int compare(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}

//vector<int> g_sfcinterval;
void treenode2sfcinterval(node nd)
{
	if (nd.width == 1) //leaf node
	{
		//int code = point2hilbertcode(nd.x0, nd.y0);
		//g_sfcinterval.push_back(code);
		//g_sfcinterval.push_back(code);

		printf("sfc interval: %d --- %d\n", code, code);
	}
	else //middle node
	{
		//get four corners, then get min and max sfc code
		int x0, y0, x1, y1;
		x0 = nd.x0; y0 = nd.y0;
		x1 = nd.x0 + nd.width - 1; //point to cell center
		y1 = nd.y0 + nd.width - 1; //point to cell center

		int sfccode[4];
		/*sfccode[0] = point2hilbertcode(x0, y0);
		sfccode[1] = point2hilbertcode(x0, y1);
		sfccode[2] = point2hilbertcode(x1, y0);
		sfccode[3] = point2hilbertcode(x1, y1);*/

		qsort(sfccode, 4, sizeof(int), compare);

		//g_sfcinterval.push_back(sfccode[0]);
		//g_sfcinterval.push_back(sfccode[3]);

		//printf("sfc interval: %d --- %d\n", sfccode[0], sfccode[3]);
	}
}


/////////////////////////////////////////////////////////////
///comparison between the tree node and the query rectangle
template< typename T, int nDims, int mBits>
int SFCQuery::query_approximate(node nd, Rectangle<T, nDims> queryrect)
{
	////this tree node now fully contains the input query rectangle
	////firstly to check this node is the leaf node: yes, stop here
	if (nd.level == mBits)
	{
		printf("node level : %d, width %d, orig (%d, %d), dest (%d, %d)\n", \
			nd.level, nd.width, nd.x0, nd.y0, nd.x0 + nd.width, nd.y0 + nd.width);

		treenode2sfcinterval(nd);
		return 0;
	}

	////check the spatial relationship between this query rectangle and four children
	node nchild[2 ^ nDims];

	for (int i = 0; i < 2 ^ nDims; i++)
	{
		nchild[i] = nd.getchildnode(i);
	}

	for (int i = 0; i < 2 ^ nDims; i++)
	{
		int nrt = nchild[i].spatialrelationship(qrt);

		if (nrt == 0)//equal to one child, that's enough, stop here
		{
			printf("node level : %d, width %d, orig (%d, %d), dest (%d, %d)\n", \
				nchild[i].level, nchild[i].width, nchild[i].x0, nchild[i].y0, \
				nchild[i].x0 + nchild[i].width, nchild[i].y0 + nchild[i].width);

			treenode2sfcinterval(nchild[i]);
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

	rect rtcut[2 * 2]; //maximum results 2*dim
	int rtpos[2 * 2] = { 0 };

	int mid[2]; // middle cut line--dim number
	mid[0] = nd.x0 + nd.width / 2;
	mid[1] = nd.y0 + nd.width / 2;

	int ncount = 1;
	rtcut[0] = qrt;
	for (int i = 0; i < 2; i++) //dimension iteration
	{
		int newadd = 0;
		for (int j = 0; j < ncount; j++)
		{
			int coord[2][2]; //2D and 2 points
			coord[0][0] = rtcut[j].x0;  //0 is less, 1 is bigger
			coord[0][1] = rtcut[j].x1;
			coord[1][0] = rtcut[j].y0;
			coord[1][1] = rtcut[j].y1;

			if (coord[i][0] < mid[i] && coord[i][1] > mid[i]) //true in the middle of this dimension
			{
				/////add one rectangle, the new rect is in the bigger area
				rect rtnew = rtcut[j];

				//cut this rectangle along the middle line
				if (i == 0) //X
				{
					rtnew.x0 = mid[i]; //right
					rtcut[j].x1 = mid[i];//left
				}
				if (i == 1) //Y
				{
					rtnew.y0 = mid[i]; //upper
					rtcut[j].y1 = mid[i];//lower
				}

				rtpos[ncount + newadd] = (1 << i) + rtpos[j]; //--put 1 on the dimension bit
				rtcut[ncount + newadd] = rtnew;
				newadd++;
			}

			if (coord[i][0] >= mid[i]) //all bigger than the middle line
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
SFCQuery::RangeQueryByRecursive(Rectangle<T, nDims> queryrect)
{
	node root;
	root.level = 0;
	root.x0 = 0;
	root.y0 = 0;
	root.width = 2 ^ mBits;

	//rect qrt;
	//qrt.x0 = 3; qrt.x1 = 6; //here,point coordidate is located on lef-bottom of the correspoind cell
	//qrt.y0 = 2; qrt.y1 = 6; //so the right-top coordidate equals cell center + 1

	/*qrt.x0 = 2; qrt.x1 = 5;
	qrt.y0 = 1; qrt.y1 = 3;*/

	///check if the root node contains or eqauls the query rectangle
	int res = root.spatialrelationship(qrt);
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

#endif