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

	double Length() const;
	const Position2D GetNormalized() const;

	static double Distance(const Position2D& p1, const Position2D& p2);
};

typedef Position2D DirVector2D;

class Degree
{
public:
	Degree(double angle)
		: angle_(angle) {}

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