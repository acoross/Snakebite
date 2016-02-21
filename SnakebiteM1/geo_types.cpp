#include "stdafx.h"
#include "geo_types.h"

double acoross::snakebite::CalcDistance(const Position2D & p1, const Position2D & p2)
{
	double xdist = (p1.x - p2.x);
	double ydist = (p1.y - p2.y);

	double sqdist = xdist*xdist + ydist*ydist;

	return sqrt(sqdist);
}
