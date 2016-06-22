//#pragma once
#ifndef POINT_H_
#define POINT_H_

#include <array>            // std::array

typedef enum
{
	Base32,
	Base64,
} StringType;

static const char* const BASE32_TABLE_E2 = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
static const char* const BASE64_TABLE_E2 = "+/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz=";

//here use the ASCII as index
static const unsigned char BASE64_TABLE_D2[] = {
	/*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/*28-2f*/ 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x01, /*2 = '+' and '/'*/
	/*30-37*/ 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, /*8 = '0'-'7'*/
	/*38-3f*/ 0x0a, 0x0b, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, /*2 = '8'-'9' and '='*/
	/*40-47*/ 0xFF, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, /*7 = 'A'-'G'*/
	/*48-4f*/ 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, /*8 = 'H'-'O'*/
	/*50-57*/ 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, /*8 = 'P'-'W'*/
	/*58-5f*/ 0x23, 0x24, 0x25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /*3 = 'X'-'Z'*/
	/*60-67*/ 0xFF, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, /*7 = 'a'-'g'*/
	/*68-6f*/ 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, /*8 = 'h'-'o'*/
	/*70-77*/ 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, /*8 = 'p'-'w'*/
	/*78-7f*/ 0x3d, 0x3e, 0x3f, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  /*3 = 'x'-'z'*/
};

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )

template< typename T, int nDims=2>
class Point
{
private:
	std::array< T, nDims > elements_;
	int dimension = nDims;
	int bitLength=64; //length means the input m or the output n

public:
	typedef T ValueType;

	T& operator[](int const i)
	{
		return elements_[i];
	}

	T const& operator[](int const i) const
	{
		return elements_[i];
	}

	void operator+=(Point const& other)
	{
		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] += other.elements_[i];
		}
	}
	

	void operator=(Point const& other)
	{
		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] = other.elements_[i];
		}
	}

	void operator-=(Point const& other)
	{
		for (int i = 0; i < nDims; ++i)
		{
			elements_[i] -= other.elements_[i];
		}
	}

	friend Point operator+(Point const& a, Point const& b)
	{
		Point ret(a);

		ret += b;
		return ret;
	}

	friend Point operator-(Point const&a, Point const& b)
	{
		Point ret(a);

		ret -= b;
		return ret;
	}

	int returnSize()
	{
		return elements_.size();
	}

	//set the input  m or the output n 
	void getBitLength(int bitLength)
	{
		this->bitLength = bitLength;
	}

	int returnBitLength()
	{
		return this->bitLength;
	}

	Point() : elements_() {}

	Point(int x, int y)
	{
		STATIC_ASSERT(nDims == 2);
		elements_[0] = x;
		elements_[1] = y;
	}

	Point(int x, int y, int z)
	{
		STATIC_ASSERT(nDims == 3);
		elements_[0] = x;
		elements_[1] = y;
		elements_[2] = z;
	}


	Point(T *coordinates)
	{
		for (int i = 0; i < this->dimension; i++)
		{
			elements_[i] = *(coordinates+i);
		}
	}

	//Point(const Point &points)
	//{
	//	this->dimension = points->returnSize();
	//	for (int i = 0; i < points->returnSize(); i++)
	//	{
	//		elements_[i] = points[i];
	//	}
	//}

	T getElements(int i)
	{ 
		return elements_[i];
	}

	T* getAllElements()
	{
		return elements_.data();
	}

	
};

typedef Point< int, 2 > Point2D;
typedef Point< int, 3 > Point3D;

#endif //POINT_H_