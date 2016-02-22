#include "stdafx.h"
#include "geo_types.h"

namespace acoross {
namespace snakebite {

double CalcLength(const DirVector2D & v)
{
	double sqdist = v.x * v.x + v.y * v.y;

	return sqrt(sqdist);
}

double CalcDistance(const Position2D & p1, const Position2D & p2)
{
	double xdist = (p1.x - p2.x);
	double ydist = (p1.y - p2.y);

	double sqdist = xdist*xdist + ydist*ydist;

	return sqrt(sqdist);
}

const Position2D Normalize(const Position2D& p)
{
	double len = CalcLength(p);

	Position2D ret = p;
	ret.x /= len;
	ret.y /= len;

	return ret;
}

}
}