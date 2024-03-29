#ifndef SNAKEBITE_GEO_TYPES_H
#define SNAKEBITE_GEO_TYPES_H

#define _USE_MATH_DEFINES
#include <math.h>

namespace acoross {
namespace snakebite {

class Position2D
{
public:
	Position2D(double ax, double ay)
		: x{ ax }, y{ ay }
	{}
	
	double Length() const
	{
		return sqrt(x * x + y * y);
	}
	const Position2D GetNormalized() const
	{
		double len = Length();

		Position2D ret = *this;
		ret.x /= len;
		ret.y /= len;

		return ret;
	}

	static double Distance(const Position2D& p1, const Position2D& p2)
	{
		double xdist = (p1.x - p2.x);
		double ydist = (p1.y - p2.y);

		double sqdist = xdist*xdist + ydist*ydist;

		return sqrt(sqdist);
	}

public:
	double x{ 0 };
	double y{ 0 };

};

typedef Position2D DirVector2D;

class Degree
{
public:
	Degree(double angle)
		: angle_(angle)
	{}

	const Degree& operator+=(const Degree& rhs)
	{
		angle_ += rhs.angle_;
		return *this;
	}

	const Degree& operator-=(const Degree& rhs)
	{
		angle_ -= rhs.angle_;
		return *this;
	}

	Degree operator+(const Degree& rhs) const
	{
		Degree ret(angle_);
		ret.Turn(rhs.angle_);
		return ret;
	}

	Degree operator-(const Degree& rhs) const
	{
		Degree ret(angle_);
		ret.Turn(-rhs.angle_);
		return ret;
	}

	double Turn(double diff)
	{
		angle_ += diff;
		int c = (int)angle_ / 360;
		angle_ = angle_ - 360 * c;

		return angle_;
	}

	void Set(double angle)
	{
		angle_ = angle;
	}

	double Get() const
	{
		return angle_;
	}

	double GetRad() const
	{
		return (double)angle_ * M_PI / 180.;
	}

private:
	double angle_;
};

class Rect final
{
public:
	Rect(int left, int top, int right, int bottom)
		: Left(left), Top(top), Right(right), Bottom(bottom)
	{}

	// game field APIs
	int Left{ 0 };
	int Right{ 0 };
	int Top{ 0 };
	int Bottom{ 0 };

	int Width() const { return Right - Left; }
	int Height() const { return Bottom - Top; }
};

template <typename T>
T Trim(T src, T minv, T maxv)
{
	_ASSERT(minv <= maxv);

	if (src < minv)
		return minv;
	if (src > maxv)
		return maxv;
	return src;
}

}
}
#endif //SNAKEBITE_GEO_TYPES_H