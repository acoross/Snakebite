#include "game_client.h"

namespace acoross {
namespace snakebite {
acoross::rpc::ErrCode SC_PushServiceImpl::QueryClientPort(
	const acoross::snakebite::messages::VoidReply & rq,
	acoross::snakebite::messages::AddressReply * rp)
{
	if (rp)
	{
		rp->set_addr(owner_->GetSocketAddress());
	}
	return acoross::rpc::ErrCode();
}

acoross::rpc::ErrCode SC_PushServiceImpl::UpdateGameObjects(
	const messages::UpdateGameObjectsEvent &rq,
	messages::VoidReply *rp)
{
	owner_->UpdateGameObjectPositions(rq);
	return acoross::rpc::ErrCode::NoError;
}

acoross::rpc::ErrCode SC_PushServiceImpl::ResetPlayer(const acoross::snakebite::messages::VoidReply &rq, acoross::snakebite::messages::VoidReply *rp)
{
	std::atomic_exchange(&owner_->player_info_, std::shared_ptr<GameClient::ClientPlayerInfo>());

	if (owner_->auto_player_mode_.load())
	{
		owner_->InitPlayer();
	}

	return acoross::rpc::ErrCode();
}

acoross::rpc::ErrCode SC_PushServiceImpl::NotifyPlayerPosition(const acoross::snakebite::messages::PlayerPosition &rq, acoross::snakebite::messages::VoidReply *rp)
{
	owner_->SetPlayerPosition(rq.idx_x(), rq.idx_y(), int(rq.x()), int(rq.y()));
	return acoross::rpc::ErrCode();
}

bool GameClient::UpdateGameObjectPositions(const messages::UpdateGameObjectsEvent& got_msg)
{
	std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>> snake_clone_list;
	std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>> apple_clone_list;

	int idx_x = got_msg.idx_x();
	int idx_y = got_msg.idx_y();

	auto clone_count = got_msg.clone_size();
	for (int i = 0; i < clone_count; ++i)
	{
		auto& clone = got_msg.clone(i);

		auto& pac_head = clone.head();
		MovingObject client_head(Position2D(pac_head.x(), pac_head.y()), pac_head.radius());

		std::list<MovingObject> client_body_list;
		for (int i = 0; i < clone.body_size(); ++i)
		{
			auto& pac_body = clone.body(i);
			client_body_list.emplace_back(Position2D(pac_body.x(), pac_body.y()), pac_body.radius());
		}

		auto client_clone = ZoneObjectClone(client_head, client_body_list, clone.obj_name(), idx_x, idx_y);

		if (clone.clone_type() == 0) //snake
		{
			snake_clone_list.emplace_back(std::make_pair(clone.handle(), client_clone));
		}
		else
		{
			apple_clone_list.emplace_back(std::make_pair(clone.handle(), client_clone));
		}
	}

	this->SetObjectList(idx_x, idx_y, std::move(snake_clone_list), std::move(apple_clone_list));

	return true;
}

}
}
