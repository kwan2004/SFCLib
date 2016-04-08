// SFCLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Point.h"

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

	return 0;
}

