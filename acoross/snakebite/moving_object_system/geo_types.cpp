#include "geo_types.h"

namespace acoross {
namespace snakebite {

double Position2D::Length() const
{
	return sqrt(x * x + y * y);
}

const Position2D Position2D::GetNormalized() const
{
	double len = Length();

	Position2D ret = *this;
	ret.x /= len;
	ret.y /= len;

	return ret;
}

//static
double Position2D::Distance(const Position2D& p1, const Position2D& p2)
{
	double xdist = (p1.x - p2.x);
	double ydist = (p1.y - p2.y);

	double sqdist = xdist*xdist + ydist*ydist;

	return sqrt(sqdist);
}

}
}