#ifndef SNAKEBITE_ZONE_OBJECT_H_
#define SNAKEBITE_ZONE_OBJECT_H_

#include <string>
#include <mutex>
#include <atomic>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {
template <typename ColliderT>
class ZoneObjectCloneEx;

template <typename ColliderT>
class GeoZone;

// ColliderT should implement bool Collide(ColliderT& other, int cnt);
template <typename ColliderT>
class ZoneObjectEx
{
public:
	using MyT = ZoneObjectEx<ColliderT>;
	using MyCloneT = ZoneObjectCloneEx<ColliderT>;

	ZoneObjectEx(ColliderT* collider, Position2D pos, double radius, std::string name = "noname")
		: collider_(collider), head_(pos, radius), Name(name), zone_idx_x_(0), zone_idx_y_(0)
	{}
	virtual ~ZoneObjectEx()
	{}

	MyCloneT Clone();

	virtual void OnEnterZoneCallback(GeoZone<ColliderT>& zone) {}
	virtual void OnLeaveZoneCallback(GeoZone<ColliderT>& zone) {}

	virtual void UpdateMove(int64_t diff_in_ms, MovingObjectContainer& container)
	{}

	bool ProcessCollision(std::shared_ptr<MyT> target)
	{
		if (target == nullptr)
		{
			_ASSERT(0);
			return false;
		}

		if (IsCollidingTo(target))
		{
			if (this->collider_)
			{
				target->collider_->Collide(*this->collider_, 0);
			}
			else
			{
				_ASSERT(0);
			}
			return true;
		}
		return false;
	}

	bool IsCollidingTo(std::shared_ptr<MyT> other) const
	{
		if (this == other.get())
		{
			return false;
		}

		if (remove_this_from_zone_.load() || other->remove_this_from_zone_.load())
		{
			return false;
		}

		if (IsCrashed(head_, other->head_))
		{
			return true;
		}

		std::lock_guard<std::recursive_mutex> lock(body_list_lock_);
		for (auto mo : other->body_list_)
		{
			if (IsCrashed(head_, mo))
			{
				return true;
			}
		}

		return false;
	}

	std::pair<int, int> AtomicZoneIdx() const
	{
		return std::make_pair(zone_idx_x_.load(), zone_idx_y_.load());
	}

	std::pair<int, int> AtomicZoneIdx(int idx_x, int idx_y)
	{
		return std::make_pair(zone_idx_x_.exchange(idx_x), zone_idx_y_.exchange(idx_y));
	}

	Position2D GetPosition() const { return head_.GetPosition(); }

	virtual void Inform()
	{}

public:
	std::atomic<int> zone_idx_x_;
	std::atomic<int> zone_idx_y_;
	std::unique_ptr<ColliderT> collider_;
	MovingObject head_;
	//@lock
	mutable std::recursive_mutex body_list_lock_;
	std::list<MovingObject> body_list_;
	//

	const std::string Name;
	std::atomic<bool> remove_this_from_zone_{ false };
};

template <typename ColliderT>
class ZoneObjectCloneEx
{
public:
	ZoneObjectCloneEx(ZoneObjectEx<ColliderT>& lhs)
		: head_(lhs.head_)
		, body_list_(lhs.body_list_)
		, Name(lhs.Name)
		, zone_idx_x_(lhs.zone_idx_x_.load())
		, zone_idx_y_(lhs.zone_idx_y_.load())
	{}

	ZoneObjectCloneEx(MovingObject head, std::list<MovingObject> body_list, std::string name, int zone_idx_x, int zone_idx_y)
		: head_(head)
		, body_list_(body_list)
		, Name(name)
		, zone_idx_x_(zone_idx_x)
		, zone_idx_y_(zone_idx_y)
	{}

	MovingObject head_;
	std::list<MovingObject> body_list_;

	const std::string Name;

	int zone_idx_x_;
	int zone_idx_y_;
};

template <typename ColliderT>
inline ZoneObjectCloneEx<ColliderT> ZoneObjectEx<ColliderT>::Clone()
{
	return ZoneObjectCloneEx<ColliderT>(*this);
}
}
}

#endif //SNAKEBITE_ZONE_OBJECT_H_