#include <string>

#include "UserSession.h"
#include "game_server.h"

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>

namespace acoross {
namespace snakebite {
void UserSession::start()
{
	std::string myid = std::to_string((uintptr_t)this);

	/*game_session_->ConnectToUpdateEvent(
		myid,
		[this, rpcsocket_wp = std::weak_ptr<rpc::RpcSocket>(shared_from_this())]
	(
		int idx_x, int idx_y,
		SbGeoZone::CloneZoneObjListT& snake_clone_list,
		SbGeoZone::CloneZoneObjListT& apple_clone_list)
	{
		if (auto rpcsocket = rpcsocket_wp.lock())
		{
			if (game_session_->GetZoneGrid().IsNeighborZone(player_idx_x_.load(), player_idx_y_.load(), idx_x, idx_y))
			{
				send_update_game_object(idx_x, idx_y, snake_clone_list, apple_clone_list);
			}
		}
	});*/

	messages::SnakebiteService::Service::start();
}

void UserSession::end()
{
	game_session_->AsyncRemoveSnake(user_snake_handle_);

	if (auto_discon_observer_to_session_)
	{
		auto_discon_observer_to_session_->disconnect();
	}
}

void UserSession::send_update_game_object(SbGeoZone::UpdateEventData ed)
{
	//int idx_x, int idx_y,
	const std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>>& snake_clone_list = *ed.mov_obj_.get();
	const std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>>& apple_clone_list = *ed.static_obj_.get();
	//int idx_x, int idx_y,
	//SbGeoZone::SharedCloneZoneObjlistT snake_clone_list
	//	SbGeoZone::SharedCloneZoneObjlistT apple_clone_list

	if (!std::atomic_load(&push_stub_))
	{
		return;
	}

	static std::atomic<int> cnt = 0;
	if (cnt.fetch_add(1) > 0)
	{
		cnt.fetch_sub(1);
		return;
	}

	messages::UpdateGameObjectsEvent game_objects;
	game_objects.set_idx_x(ed.idx_x_);
	game_objects.set_idx_y(ed.idx_y_);
	{
		for (auto& pair : snake_clone_list)
		{
			auto& snake_clone = pair.second;
			auto* clone = game_objects.add_clone();

			clone->set_clone_type(0);
			clone->set_handle(Handle<Snake>(pair.first).handle);
			clone->set_obj_name(snake_clone.Name);

			auto* head = clone->mutable_head();
			head->set_radius(snake_clone.head_.GetRadius());
			head->set_x(snake_clone.head_.GetPosition().x);
			head->set_y(snake_clone.head_.GetPosition().y);

			for (auto& body : snake_clone.body_list_)
			{
				auto* pac_body = clone->add_body();
				pac_body->set_radius(body.GetRadius());
				pac_body->set_x(body.GetPosition().x);
				pac_body->set_y(body.GetPosition().y);
			}
		}

		for (auto& pair : apple_clone_list)
		{
			auto& apple_clone = pair.second;
			auto* clone = game_objects.add_clone();

			clone->set_clone_type(1);
			clone->set_handle(pair.first);
			clone->set_obj_name(apple_clone.Name);

			auto* head = clone->mutable_head();
			head->set_radius(apple_clone.head_.GetRadius());
			head->set_x(apple_clone.head_.GetPosition().x);
			head->set_y(apple_clone.head_.GetPosition().y);

			for (auto& body : apple_clone.body_list_)
			{
				auto* pac_body = clone->add_body();
				pac_body->set_radius(body.GetRadius());
				pac_body->set_x(body.GetPosition().x);
				pac_body->set_y(body.GetPosition().y);
			}
		}
	}

	if (bool is_initialized = game_objects.IsInitialized())
	{
		if (auto push = std::atomic_load(&push_stub_))
		{
			push->UpdateGameObjects(game_objects,
				[this](rpc::ErrCode ec, messages::VoidReply&)
			{
				//cnt.fetch_sub(1);
			});
			cnt.fetch_sub(1);
		}
	}
}

//////////////////////////////////////
// rpc methods

acoross::rpc::ErrCode UserSession::RequestZoneInfo(const acoross::snakebite::messages::VoidReply &rq, acoross::snakebite::messages::ZoneInfoReply *rp)
{
	const auto& zone_grid = game_session_->GetZoneGrid();
	rp->set_limit_idx_x(zone_grid.N_X);
	rp->set_limit_idx_y(zone_grid.N_Y);
	rp->set_height(zone_grid.ZoneHeight);
	rp->set_width(zone_grid.ZoneWidth);

	rp->set_my_address(my_address_.c_str());

	return acoross::rpc::ErrCode();
}

acoross::rpc::ErrCode UserSession::InitPlayer(const acoross::snakebite::messages::InitPlayerSnakeRequest &rq, acoross::snakebite::messages::InitPlayerSnakeReply *rp)
{
	game_session_->AsyncRemoveSnake(user_snake_handle_);

	auto self_wp = std::weak_ptr<RpcSocket>(shared_from_this());
	user_snake_handle_ = game_session_->AsyncMakeNewSnake(rq.name(),
		[self_wp, this](Snake& snake)	//die callback
	{
		if (auto self = self_wp.lock())
		{
			user_snake_handle_ = 0;
			if (auto push = std::atomic_load(&push_stub_))
			{
				messages::VoidReply vr;
				vr.set_err(0);

				push->ResetPlayer(vr,
					[](rpc::ErrCode ec, messages::VoidReply&)
				{});
			}
		}
	}, /*is_connect_zone = */ true);

	auto prom_sp = std::make_shared<boost::promise<acoross::auto_connection>>();
	auto fut = prom_sp->get_future();

	auto prom_set_mov_event_sp = std::make_shared<boost::promise<acoross::auto_connection>>();
	auto fut_mov_event = prom_set_mov_event_sp->get_future();

	game_session_->RequestToSnake(
		user_snake_handle_,
		[this, self_wp, prom_sp, prom_set_mov_event_sp](Snake& snake) mutable
	{
		auto auto_conn = snake.ConnectToUpdateEventRelayer(
			[this, self_wp](SbGeoZone::UpdateEventData ed)
		{
			if (auto rpcsocket = self_wp.lock())
			{
				//if (game_session_->GetZoneGrid().IsNeighborZone(player_idx_x_.load(), player_idx_y_.load(), idx_x, idx_y))
				{
					send_update_game_object(ed);
				}
			}
		});

		prom_sp->set_value(std::move(auto_conn));

		auto auto_conn_2_mov_event = snake.ConnectToPositionUpdateEvent(
			[this, self_wp](int idx_x, int idx_y,
				double pos_x, double pos_y)
		{
			if (auto rpcsocket = self_wp.lock())
			{
				if (auto push = std::atomic_load(&push_stub_))
				{
					messages::PlayerPosition rq;
					rq.set_idx_x(idx_x);
					rq.set_idx_y(idx_y);
					rq.set_x(pos_x);
					rq.set_y(pos_y);

					push->NotifyPlayerPosition(rq, [](acoross::rpc::ErrCode, messages::VoidReply& rp) {});
				}
			}
		});

		prom_set_mov_event_sp->set_value(std::move(auto_conn_2_mov_event));
	});

	auto_discon_observer_to_session_ = std::move(fut.get());
	auto_discon_observer_to_player_ = std::move(fut_mov_event.get());

	if (rp)
	{
		rp->set_handle(user_snake_handle_);
	}

	return acoross::rpc::ErrCode();
}

acoross::rpc::ErrCode UserSession::SetKeyDown(const acoross::snakebite::messages::TurnKeyDownRequest &rq, acoross::snakebite::messages::VoidReply *rp)
{
	game_session_->RequestToSnake(user_snake_handle_,
		[pk = rq.key()](Snake& snake)
	{
		snake.SetKeyDown(PlayerKey(pk));
	});

	return acoross::rpc::ErrCode();
}

acoross::rpc::ErrCode UserSession::SetKeyUp(const acoross::snakebite::messages::TurnKeyUpRequest &rq, acoross::snakebite::messages::VoidReply *rp)
{
	game_session_->RequestToSnake(user_snake_handle_,
		[pk = rq.key()](Snake& snake)
	{
		snake.SetKeyUp(PlayerKey(pk));
	});

	return acoross::rpc::ErrCode();
}
}
}