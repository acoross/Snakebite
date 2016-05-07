#include "snake.h"
#include "snake_collider.h"
#include "game_session.h"

namespace acoross {
namespace snakebite {

SnakeNode::SnakeNode(GameSession& game_session,
	HandleT owner,
	SbColliderBase* collider, const Position2D & pos, double radius, std::string name)
	: SbZoneObject(collider, pos, radius, name)
	, game_session_(game_session)
	, owner_handle_(owner)
{}

SnakeNode::~SnakeNode()
{
	if (auto next = std::atomic_load(&next_))
	{
		next->remove_this_from_zone_.store(true);
	}
}

// snake 를 지정한 diff_vec 만큼 이동시킨다.
Position2D SnakeNode::Move(const DirVector2D & diff_vec, Rect & boundary)
{
	const double dist_mov = diff_vec.Length();

	Position2D pos_prev_node = head_.GetPosition();
	head_.Move(diff_vec, boundary);

	//DirVector2D diff_prev = diff_vec;

	std::lock_guard<std::recursive_mutex> lock(body_list_lock_);
	for (auto& mo : body_list_)
	{
		const Position2D pos_current_node = mo.GetPosition();
		const DirVector2D diff_now_to_prev = {
			pos_prev_node.x - pos_current_node.x,
			pos_prev_node.y - pos_current_node.y
		};

		const double limitdist = 2 * (mo.GetRadius()) * 0.9;
		if (diff_now_to_prev.Length() < limitdist)
		{
			break;
		}

		auto diff_move = diff_now_to_prev.GetNormalized();
		diff_move.x *= dist_mov;
		diff_move.y *= dist_mov;

		pos_prev_node = mo.GetPosition();
		mo.Move(diff_move, boundary);
	}

	return pos_prev_node;
}

//@lock
void SnakeNode::AddBody()
{
	std::lock_guard<std::recursive_mutex> lock(body_list_lock_);

	if (body_list_.size() < max_internal_body_len_)
	{
		body_list_.emplace_back(head_.GetPosition(), head_.GetRadius());
		return;
	}
	else
	{
		if (auto next = std::atomic_load(&next_))
		{
			next->AddBody();
		}
		else
		{
			auto new_next = std::make_shared<SnakeTail>(game_session_,
				owner_handle_,	// 자기자신이 owner 인 경우도 있음.
				head_.GetPosition(), head_.GetRadius());

			std::shared_ptr<SnakeNode> exp(nullptr);
			std::atomic_compare_exchange_strong(
				&next_,
				&exp,
				(std::shared_ptr<SnakeNode>)new_next);
			game_session_.AsyncAddSnakeTail(new_next);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////
// SnakeTail
SnakeTail::SnakeTail(GameSession& game_session, HandleT owner, const Position2D & pos, double radius)
	: SnakeNode(game_session,
		owner,
		new SnakeTailCollider(this), pos, radius, "t")
{}

SnakeTail::~SnakeTail()
{}

// snake 가 알아서 움직인다.
void SnakeTail::UpdateMove(int64_t diff_in_ms, Rect & boundary)
{
	if (auto prev_pos = std::atomic_load(&prev_last_pos_))
	{
		auto cur_pos = head_.GetPosition();
		DirVector2D diff_vec{
			prev_pos->x - cur_pos.x,
			prev_pos->y - cur_pos.y
		};

		auto len = diff_vec.Length();
		if (len > head_.GetRadius())
		{
			diff_vec = diff_vec.GetNormalized();
			diff_vec.x *= (len * 0.9);
			diff_vec.y *= (len * 0.9);
		}

		auto last_moved_pos = Move(diff_vec, boundary);

		if (auto next = std::atomic_load(&next_))
		{
			next->SetLastPrevPosition(std::make_shared<Position2D>(last_moved_pos));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
// Snake

Snake::Snake(GameSession& game_session, const Position2D & pos, double radius,
	const Degree & angle, double velocity, double ang_vel, int len,
	EventHandler onDie, std::string name,
	bool is_connect_to_zone)
	: SnakeNode(game_session,
		Handle<Snake>(this).handle,
		new SnakeCollider(this), pos, radius, name)
	, angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
	, onDie_(onDie)
	, is_connect_to_zone_(is_connect_to_zone)
	, update_event_relayer_(std::make_shared<SbGeoZone::UpdateEventRelayer>())
{
	for (int i = 0; i < len; ++i)
	{
		AddBody();
	}
}

Snake::~Snake()
{

}

void Snake::UpdateMove(int64_t diff_in_ms, Rect& boundary)
{
	// change direction
	{
		auto last_pk = GetPlayerKey();
		if (last_pk == PK_RIGHT)
		{
			auto ang_vel = GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			Turn(diff_ang);
		}
		else if (last_pk == PK_LEFT)
		{
			auto ang_vel = GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			Turn(diff_ang);
		}
	}

	// move to forward
	double diff_distance = GetVelocity() * diff_in_ms;
	Position2D pos_now = GetPosition();
	double angle_now_rad = GetAngle().GetRad();

	DirVector2D diff_vec{
		diff_distance * std::cos(angle_now_rad),
		diff_distance * std::sin(angle_now_rad)
	};

	auto last_moved_pos = Move(diff_vec, boundary);

	if (auto next = std::atomic_load(&next_))
	{
		next->SetLastPrevPosition(std::make_shared<Position2D>(last_moved_pos));
	}

	auto& pos_after = GetPosition();
	mov_event_.invoke(0, 0, pos_after.x, pos_after.y);
}

//@thread-safe: atomic once
void Snake::Die()
{
	bool expAlive = true;
	if (isAlive_.compare_exchange_strong(expAlive, false))
	{
		if (onDie_)
		{
			onDie_(*this);
		}

		game_session_.AsyncRemoveSnake(Handle<Snake>(this).handle);

		if (auto next = std::atomic_load(&next_))
		{
			next->remove_this_from_zone_.store(true);
		}
	}
}

}
}