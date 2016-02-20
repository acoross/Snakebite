#ifndef SNAKEBITE_GEO_TYPES_H
#define SNAKEBITE_GEO_TYPES_H

namespace acoross {
namespace snakebite {

struct Position2D
{
	double x;
	double y;
};

typedef Position2D DirVector2D;

class Degree
{
public:
	Degree(int angle)
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

	int Turn(int diff)
	{
		angle_ += diff;
		angle_ %= 360;

		return angle_;
	}

	void Set(int angle)
	{
		angle_ = angle;
	}

	int Get() const
	{
		return angle_;
	}
private:
	int angle_;
};

}
}

#endif //SNAKEBITE_GEO_TYPES_H