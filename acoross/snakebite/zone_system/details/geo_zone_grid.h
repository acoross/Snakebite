#ifndef ACOROSS_GAME_GEO_ZONE_GRID_H_
#define ACOROSS_GAME_GEO_ZONE_GRID_H_

#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <mutex>
#include <vector>
#include <queue>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

///////////////////////////////////////////////////////////////////

// GeoZone 에 대한 const container
// @thread-safe: containing-geo zone 에 대한 접근은 constant 하므로 thread-safe
//	내용물은 geo zone 의 멤버함수 호출은 geo zone 구현에 따라 다름.
template <typename ZoneT>
class ZoneGrid final
{
public:
	/////////////////////////////////////////////
	// GameGeoZoneGrid
	ZoneGrid(::boost::asio::io_service& io_service,
		int zone_width, int zone_height, int n_x, int n_y)
		: game_boundary_(0, n_x * zone_width, 0, n_y * zone_height)
		, ZoneWidth(zone_width), ZoneHeight(zone_height)
		, N_X(n_x), N_Y(n_y)
	{
		for (int i = 0; i < N_X; ++i)
		{
			int zone_left = i * zone_width;
			zone_grid_.push_back(std::vector<std::shared_ptr<ZoneT>>());

			for (int j = 0; j < N_Y; ++j)
			{
				int zone_top = j * zone_height;
				zone_grid_[i].emplace_back(
					std::make_shared<ZoneT>
					(
						io_service,
						*this,
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
	~ZoneGrid() {};

	bool ProcessAllZone(std::function<bool(ZoneT&)> func, bool stop_condition = false)
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
	// x, y: 좌표
	// return: 해당 위치의 zone
	std::shared_ptr<ZoneT> get_zone(double x, double y)
	{
		auto idx_x = int(x / ZoneWidth);
		auto idx_y = int(y / ZoneHeight);

		return get_zone_by_idx(idx_x, idx_y);
	}
	// idx_x, idx_y: grid 인덱스
	std::shared_ptr<ZoneT> get_zone_by_idx(int idx_x, int idx_y)
	{
		if (IsValidIdx(idx_x, idx_y) == false)
		{
			return std::shared_ptr<ZoneT>();
		}

		return zone_grid_[idx_x][idx_y];
	}
	bool IsValidIdx(int idx_x, int idx_y) const
	{
		bool is_x_valid = (idx_x >= 0) && (idx_x < N_X);
		bool is_y_valid = (idx_y >= 0) && (idx_y < N_Y);

		if (!is_x_valid || !is_y_valid)
			return false;

		return true;
	}
	bool IsNeighborZone(int src_idx_x, int src_idx_y, int tar_idx_x, int tar_idx_y) const
	{
		if (IsValidIdx(src_idx_x, src_idx_y) == false)
			return false;

		if (IsValidIdx(tar_idx_x, tar_idx_y) == false)
			return false;

		auto diff_x = src_idx_x - tar_idx_x;
		auto diff_y = src_idx_y - tar_idx_y;
		if (diff_x >= -1 && diff_x <= 1
			&& diff_y >= -1 && diff_y <= 1)
		{
			return true;
		}

		return false;
	}

	MovingObjectContainer& GetBoundaryContainer()
	{
		return game_boundary_;
	}
	
	const int N_X;
	const int N_Y;

	const int ZoneWidth;
	const int ZoneHeight;

private:
	std::vector<std::vector<std::shared_ptr<ZoneT>>> zone_grid_;
	MovingObjectContainer game_boundary_;
};

}
}

#include "geo_zone_grid.ipp"

#endif //ACOROSS_GAME_GEO_ZONE_GRID_H_