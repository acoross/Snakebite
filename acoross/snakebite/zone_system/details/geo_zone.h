#ifndef SNAKEBITE_GAME_GEO_ZONE_H_
#define SNAKEBITE_GAME_GEO_ZONE_H_

#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <unordered_map>
#include <list>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

#include "signal.h"
#include "handle.h"
#include "geo_zone_grid.h"
#include "zone_object.h"

#include <acoross/snakebite/util.h>

namespace acoross {
namespace snakebite {

//@need to be serialized
// 벽에 충돌하면 튕겨나옴.
// BUG!!!
// when move to neighbor zone, actor should be removed from wall_collision_set_, but now it doesn't
template <typename ColliderT>
static void reflect_object_at_boundary(std::shared_ptr<ZoneObjectEx<ColliderT>> actor, Rect& boundary)
{
	if (pos.x <= game_boundary_.Left + 1 || pos.x >= game_boundary_.Right - 1
		|| pos.y <= game_boundary_.Top + 1 || pos.y >= game_boundary_.Bottom - 1)
	{
		pos.x = Trim((int)pos.x, game_boundary_.Left + 1, game_boundary_.Right - 1);
		auto ret = wall_collision_set_.insert(Handle<Snake>(actor.get()).handle);
		if (ret.second == true)
		{
			// onCollideBegin
			if (pos.x <= game_boundary_.Left + 1 || pos.x >= game_boundary_.Right - 1)
			{
				actor->SetAngle(180. - actor->GetAngle().Get());
			}
			else if (pos.y <= game_boundary_.Top + 1 || pos.y >= game_boundary_.Bottom - 1)
			{
				actor->SetAngle(-1 * actor->GetAngle().Get());
			}
		}
		else
		{
			// onColliding
		}
	}
	else
	{
		if (wall_collision_set_.erase(Handle<Snake>(actor.get()).handle) > 0)
		{
			// onCollideEnd
		}
	}
}

//@need to be serialized
// boundary 벗어나지 않게 조정
template <typename ColliderT>
static void trim_object_position(std::shared_ptr<ZoneObjectEx<ColliderT>> actor, Rect& boundary)
{
	auto& pos = actor->GetPosition();
	pos.x = Trim((int)pos.x, boundary.Left + 1, boundary.Right - 1);
	pos.y = Trim((int)pos.y, boundary.Top + 1, boundary.Bottom - 1);
}

//@need to be serialized
// boundary 벗어나지 않게 조정
template <typename ColliderT>
static void process_collision_to_wall(std::shared_ptr<ZoneObjectEx<ColliderT>> actor, Rect& boundary)
{
	trim_object_position(actor, boundary);
}

//////////////////////////////////////////////////
// 단일 게임을 여러개의 zone 으로 쪼개어 쓴다.

// serialized by strand
template <typename ColliderT>
class GeoZone
	: std::enable_shared_from_this<GeoZone<ColliderT>>
{
public:
	using MyT = GeoZone<ColliderT>;
	using GameGeoZoneGrid = ZoneGrid<MyT>;

	using MyObjT = ZoneObjectEx<ColliderT>;
	using MapMyObj = std::unordered_map<HandleT, std::shared_ptr<MyObjT>>;

	using MyCloneObjT = ZoneObjectCloneEx<ColliderT>;
	using CloneZoneObjListT = std::list<std::pair<HandleT, MyCloneObjT>>;
	using SharedCloneZoneObjlistT = std::shared_ptr<CloneZoneObjListT>;
	
	class UpdateEventData
	{
	public:
		UpdateEventData(
			int idx_x,
			int idx_y,
			SharedCloneZoneObjlistT mov_obj,
			SharedCloneZoneObjlistT static_obj)
			: idx_x_(idx_x), idx_y_(idx_y), mov_obj_(mov_obj), static_obj_(static_obj)
		{}
		UpdateEventData(const UpdateEventData& other)
			: idx_x_(other.idx_x_), idx_y_(other.idx_y_), mov_obj_(other.mov_obj_), static_obj_(other.static_obj_)
		{}
		UpdateEventData& operator=(const UpdateEventData& other) = default;

		int idx_x_;
		int idx_y_;
		SharedCloneZoneObjlistT mov_obj_;
		SharedCloneZoneObjlistT static_obj_;
	};

	using ObserverT = std::function<void(UpdateEventData)>;
	using UpdateEvent = acoross::Event<void(UpdateEventData)>;
	using UpdateEventRelayer = acoross::EventRelayer<void(UpdateEventData)>;

	/////////////////////////////////////////////////
	// GeoZone
	explicit GeoZone(
		::boost::asio::io_service& io_service,
		GameGeoZoneGrid& owner_zone_grid,
		int idx_zone_x, int idx_zone_y,
		Rect& game_boundary,
		Rect zone_boundary)
		: strand_(io_service)
		, zone_timer_(io_service)
		, owner_zone_grid_(owner_zone_grid)
		, IDX_ZONE_X(idx_zone_x)
		, IDX_ZONE_Y(idx_zone_y)
		, game_boundary_(game_boundary)
		, zone_boundary_(zone_boundary)
	{}
	~GeoZone()
	{}

	void Init()
	{}

	// timer 를 이용하는 update loop 를 시작한다.
	// loop 는 strand 를 통해 serialize 된다.
	void Run(int frame_tick)
	{
		bool exp = false;
		if (is_running_.compare_exchange_strong(exp, true))
		{
			zone_timer_tick_ = frame_tick;
			AsyncDoUpdate();
		}
	}

	UpdateEvent& GetUpdateEvent()
	{
		return update_event_;
	}

	// 움직이지 않는 object 추가
	void AsyncAddStaticObj(std::shared_ptr<MyObjT> static_obj)
	{
		strand_.post(
			[this, static_obj]()
		{
			static_objects_.emplace(std::make_pair(Handle<MyObjT>(static_obj.get()).handle, static_obj));
			cached_static_object_cnt_.fetch_add(1);
			static_obj->AtomicZoneIdx(IDX_ZONE_X, IDX_ZONE_Y);
		});
	}
	// 움직이지 않는 object 제거
	void AsyncRemoveStaticObj(HandleT handle, std::function<void(bool result)> callback)
	{
		strand_.post(
			[&, this, handle, callback]()
		{
			auto it = static_objects_.find(handle);
			if (it != static_objects_.end())
			{
				it->second->AtomicZoneIdx(0, 0);

				static_objects_.erase(it);
				cached_static_object_cnt_.fetch_sub(1);

				return callback(true);
			}

			return callback(false);
		});
	}
	// 움직이는 object 추가
	void AsyncAddMovObj(std::shared_ptr<MyObjT> mov_obj)
	{
		strand_.post(
			[this, mov_obj]()
		{
			if (mov_obj->remove_this_from_zone_.load())
			{
				mov_obj->AtomicZoneIdx(0, 0);
				return;
			}

			auto it = mov_objects_.emplace(Handle<MyObjT>(mov_obj.get()).handle, mov_obj);
			if (it.second)
			{
				it.first->second->AtomicZoneIdx(IDX_ZONE_X, IDX_ZONE_Y);
				it.first->second->OnEnterZoneCallback(*this);
				cached_mov_object_cnt_.fetch_add(1);
			}
		});
	}

	size_t AtomicStaticObjCount() const
	{
		return cached_static_object_cnt_.load();
	}
	size_t AtomicMovObjCount() const
	{
		return cached_mov_object_cnt_.load();
	}

	// 다른 zone 의 정보를 내 zone 의 observer 에게 보내는 method
	void AsyncInvokeUpdateEventsToObservers(UpdateEventData ed)
	{
		strand_.post(
			[this, ed]()
		{
			invoke_update_event_to_observers(ed);
		});
	}

public:
	const int IDX_ZONE_X;
	const int IDX_ZONE_Y;

	//
	std::atomic<double> mean_update_time_ms_{ 0 };
	std::atomic<double> mean_broadcast_time_ms_{ 0 };
	//

private:
	// update loop, using timer
	void AsyncDoUpdate()
	{
		strand_.post(
			[this]()
		{
			MeanProcessTimeChecker mean_update(mean_update_time_ms_);

			zone_timer_.expires_from_now(boost::posix_time::milliseconds(zone_timer_tick_));

			auto cnt = mov_objects_.size();
			update_movobj_position(zone_timer_tick_);

			cnt = mov_objects_.size();
			process_collision(owner_zone_grid_);

			cnt = mov_objects_.size();
			invoke_my_update_event_to_observers();

			zone_timer_.async_wait(
				[this](boost::system::error_code ec)
			{
				if (!ec)
				{
					AsyncDoUpdate();
				}
				else
				{
					throw(std::exception("zone update error"));
				}
			});
		});
	}

	// 다른 존하고의 충돌체크
	// 다른 존하고의 충돌체크
	void AsyncProcessCollisionTo(
		std::shared_ptr<MapMyObj> shared_other_snakes)
	{
		strand_.post(
			[this, shared_other_snakes]()
		{
			if (mov_objects_.empty() && static_objects_.empty())
			{
				return;
			}

			process_collision_2_mapsnake(
				*shared_other_snakes,
				mov_objects_,
				static_objects_);
		});
	}

	// @use in serializer
	// update every snake position
	void update_movobj_position(int64_t diff_in_ms)
	{
		std::list<Handle<MyObjT>::Type> del_list;

		for (auto& pair : mov_objects_)
		{
			auto& mov_obj = pair.second;

			if (mov_obj->remove_this_from_zone_.load())
			{
				mov_obj->AtomicZoneIdx(0, 0);
				mov_obj->OnLeaveZoneCallback(*this);
				cached_mov_object_cnt_.fetch_sub(1);
				del_list.push_back(pair.first);
			}
			else
			{
				mov_obj->UpdateMove(diff_in_ms, game_boundary_);
				//process_collision_to_wall((std::shared_ptr<MyObjT>)mov_obj, game_boundary_);
			}
		}

		for (auto& handle : del_list)
		{
			mov_objects_.erase(handle);
		}
	}

	// @use in serializer
	// process collision with neighbor zones
	void process_collision(GameGeoZoneGrid& neighbors)
	{
		auto shared_src_mov_objs = std::make_shared<MapMyObj>(mov_objects_);
		for (auto& pair : *shared_src_mov_objs)
		{
			auto& snake = pair.second;
			auto& pos = snake->GetPosition();
			auto width = zone_boundary_.Width();
			auto height = zone_boundary_.Height();

			/*if (pos.x < zone_boundary_.Left - width / 4
			|| pos.x >= zone_boundary_.Right + width / 4
			|| pos.y < zone_boundary_.Top - height / 4
			|| pos.y >= zone_boundary_.Bottom + height / 4)*/
			if (pos.x < zone_boundary_.Left
				|| pos.x >= zone_boundary_.Right
				|| pos.y < zone_boundary_.Top
				|| pos.y >= zone_boundary_.Bottom)
			{
				auto& dest_zone = owner_zone_grid_.get_zone(pos.x, pos.y);
				if (dest_zone)
				{
					auto it = mov_objects_.find(pair.first);
					if (it != mov_objects_.end())
					{
						it->second->AtomicZoneIdx(0, 0);
						it->second->OnLeaveZoneCallback(*this);
						mov_objects_.erase(it);
						//del_list.push_back(pair.first);
						cached_mov_object_cnt_.fetch_sub(1);
					}
					dest_zone->AsyncAddMovObj(snake);
				}
			}
		}

		//// snake 를 한 존에서만 접근하려고 수정...
		this->AsyncProcessCollisionTo(shared_src_mov_objs);
		/*for (int x = -1; x <= 1; ++x)
		{
		for (int y = -1; y <= 1; ++y)
		{
		auto neighbor_zone = neighbors.get_zone_by_idx(IDX_ZONE_X + x, IDX_ZONE_Y + y);
		if (neighbor_zone)
		{
		neighbor_zone->AsyncProcessCollisionTo(shared_src_mov_objs);
		}
		}
		}*/
	}

	// @use in serializer
	// zone 내부의 objects 위치를 broadcast
	void invoke_my_update_event_to_observers()
	{
		auto snakes = std::make_shared<CloneZoneObjListT>();
		for (auto pair : mov_objects_)
		{
			if (pair.second->remove_this_from_zone_.load() == false)
			{
				snakes->push_back(std::make_pair(pair.first, pair.second->Clone()));
			}
		}

		auto apples = std::make_shared<CloneZoneObjListT>();
		for (auto apple : static_objects_)
		{
			apples->push_back(std::make_pair(apple.first, apple.second->Clone()));
		}

		UpdateEventData ed(IDX_ZONE_X, IDX_ZONE_Y, snakes, apples);

		// event to me
		invoke_update_event_to_observers(ed);

		// broadcast to neighbors
		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				if (x == 0 && y == 0)
					continue;

				auto neighbor_zone = owner_zone_grid_.get_zone_by_idx(IDX_ZONE_X + x, IDX_ZONE_Y + y);
				if (neighbor_zone)
				{
					neighbor_zone->AsyncInvokeUpdateEventsToObservers(ed);
				}
			}
		}
	}

	// refactoring 필요
	// @use in serializer
	void invoke_update_event_to_observers(UpdateEventData ed)
	{
		MeanProcessTimeChecker time_checker(mean_broadcast_time_ms_);
		update_event_.invoke(ed);
	}

	static void process_collision_2_mapsnake(MapMyObj& src_mov_objs, MapMyObj& target_mov_objs, MapMyObj& static_objs)
	{
		for (auto& mov_obj1 : src_mov_objs)
		{
			for (auto& mov_obj2 : target_mov_objs)
			{
				mov_obj1.second->ProcessCollision(mov_obj2.second);
			}

			for (auto& static_obj : static_objs)
			{
				mov_obj1.second->ProcessCollision(static_obj.second);
			}
		}
	}

private:
	::boost::asio::strand strand_;
	::boost::asio::deadline_timer zone_timer_;

	int zone_timer_tick_{ 100 };
	std::atomic<bool> is_running_{ false };

	GameGeoZoneGrid& owner_zone_grid_;
	Rect zone_boundary_;
	Rect& game_boundary_;

	//@use in strand_
	UpdateEvent update_event_;
	MapMyObj mov_objects_;
	MapMyObj static_objects_;
	//

	// monitor
	std::atomic<int> cached_mov_object_cnt_{ 0 };
	std::atomic<int> cached_static_object_cnt_{ 0 };
};
//

}
}
#endif //SNAKEBITE_GAME_GEO_ZONE_H_