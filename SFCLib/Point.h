#pragma once
#ifndef POINT_H_
#define POINT_H_

#include <array>            // std::array

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )

template< typename T, int nDimensions=2>
class Point
{
private:
	std::array< T, nDimensions > elements_;
	int dimension=nDimensions;
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
		for (int i = 0; i < nDimensions; ++i)
		{
			elements_[i] += other.elements_[i];
		}
	}


	void operator=(Point const& other)
	{
		for (int i = 0; i < nDimensions; ++i)
		{
			elements_[i] = other.elements_[i];
		}
	}

	void operator-=(Point const& other)
	{
		for (int i = 0; i < nDimensions; ++i)
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
		STATIC_ASSERT(nDimensions == 2);
		elements_[0] = x;
		elements_[1] = y;
	}

	Point(int x, int y, int z)
	{
		STATIC_ASSERT(nDimensions == 3);
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

	T getElements(int i)
	{ 
		return elements_[i];
	}

	
};

typedef Point< int, 2 > Point2D;
typedef Point< int, 3 > Point3D;

#endif //POINT_H_