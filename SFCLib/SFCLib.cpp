// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Point.h"
#include "Rectangle.h"

#include <iostream>
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	cout << "starting" << endl;
	Point3D a(1, 2, 3);
	Point3D b(4, 5, 6);

	a += b;

	a[0] = 5;

	//cout << a << endl;

	int c = a[0];
	long points[5] = { 1, 2, 3, 4, 5 };
	Point<long, 5> point5D(points);
	for (int i = 0; i < 5; i++)
	{
		printf("%d", point5D[i]);
	}

	long Point1[3] = { 1, 2 ,3};
	long Point2[3] = { 4,5,6 };
	Point<long, 3> MinPoint(Point1);
	Point<long, 3> MaxPoint(Point1);
	Rectangle<long, 3> rec(MinPoint, MaxPoint);
	rec.generateComplateRec(MinPoint, MaxPoint);

	system("pause");
	return 0;
}

