#ifndef ACOROSS_GAME_GEO_ZONE_GRID_H_
#define ACOROSS_GAME_GEO_ZONE_GRID_H_

#include <mutex>
#include <vector>
#include <queue>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "game_geo_zone_define.h"

namespace acoross {
namespace snakebite {

///////////////////////////////////////////////////////////////////
class GameGeoZone;

class GameGeoZoneGrid final
{
public:
	GameGeoZoneGrid(int zone_width, int zone_height, int n_x, int n_y);
	~GameGeoZoneGrid() {};

	bool ProcessAllZone(std::function<bool(GameGeoZone&)> func, bool stop_condition = false);

	// x, y: ÁÂÇ¥
	// return: ÇØ´ç À§Ä¡ÀÇ zone
	std::shared_ptr<GameGeoZone> get_zone(double x, double y);
	// idx_x, idx_y: grid ÀÎµ¦½º
	std::shared_ptr<GameGeoZone> get_zone_by_idx(int idx_x, int idx_y);

	MovingObjectContainer& GetBoundaryContainer()
	{
		return game_boundary_;
	}
	
	const int N_X;
	const int N_Y;

	const int ZoneWidth;
	const int ZoneHeight;

private:
	std::vector<std::vector<std::shared_ptr<GameGeoZone>>> zone_grid_;
	MovingObjectContainer game_boundary_;
};

}
}
#endif //ACOROSS_GAME_GEO_ZONE_GRID_H_