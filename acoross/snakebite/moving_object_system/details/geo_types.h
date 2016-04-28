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

	double x{ 0 };
	double y{ 0 };

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

}
}
#endif //SNAKEBITE_GEO_TYPES_H