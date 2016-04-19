#include "game_geo_zone_grid.h"
#include "game_geo_zone.h"

namespace acoross {
namespace snakebite {

/////////////////////////////////////////////
// GameGeoZoneGrid
GameGeoZoneGrid::GameGeoZoneGrid(
	::boost::asio::io_service& io_service, 
	int zone_width, int zone_height, int n_x, int n_y)
	: game_boundary_(0, n_x * zone_width, 0, n_y * zone_height)
	, ZoneWidth(zone_width), ZoneHeight(zone_height)
	, N_X(n_x), N_Y(n_y)
{
	for (int i = 0; i < N_X; ++i)
	{
		int zone_left = i * zone_width;
		zone_grid_.push_back(std::vector<std::shared_ptr<GameGeoZone>>());

		for (int j = 0; j < N_Y; ++j)
		{
			int zone_top = j * zone_height;
			zone_grid_[i].emplace_back(
				std::make_shared<GameGeoZone>
				(
					io_service,
					i, j,
					game_boundary_,
					zone_left,
					zone_top,
					zone_width,
					zone_height
				)
			);
		}
	}
}

bool GameGeoZoneGrid::ProcessAllZone(std::function<bool(GameGeoZone&)> func, bool stop_condition)
{
	for (auto& zone_x_arr : zone_grid_)
	{
		for (auto& zone : zone_x_arr)
		{
			if (zone)
			{
				if (func(*zone) == stop_condition)
				{
					return stop_condition;
				}
			}
		}
	}
	return !stop_condition;
}

// x, y: 좌표
// return: 해당 위치의 zone
std::shared_ptr<GameGeoZone> GameGeoZoneGrid::get_zone(double x, double y)
{
	auto idx_x = int(x / ZoneWidth);
	auto idx_y = int(y / ZoneHeight);

	return get_zone_by_idx(idx_x, idx_y);
}

std::shared_ptr<GameGeoZone> GameGeoZoneGrid::get_zone_by_idx(int idx_x, int idx_y)
{
	bool is_x_valid = (idx_x >= 0) && (idx_x < N_X);
	bool is_y_valid = (idx_y >= 0) && (idx_y < N_Y);

	//_ASSERT(is_x_valid && is_y_valid);
	if (!is_x_valid || !is_y_valid)
	{
		return std::shared_ptr<GameGeoZone>();
	}

	return zone_grid_[idx_x][idx_y];
}

}
}
