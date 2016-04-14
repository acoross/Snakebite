#include "game_client.h"

namespace acoross {
namespace snakebite {

bool GameClient::UpdateGameObjectPositions(messages::UpdateGameObjectsEvent& got_msg)
{
	std::list<std::pair<Handle<Snake>::Type, GameObjectClone>> snake_clone_list;
	std::list<GameObjectClone> apple_clone_list;

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

		auto client_clone = GameObjectClone(client_head, client_body_list, clone.obj_name());

		if (clone.clone_type() == 0) //snake
		{
			snake_clone_list.emplace_back(std::make_pair(clone.handle(), client_clone));
		}
		else
		{
			apple_clone_list.emplace_back(client_clone);
		}
	}

	this->SetObjectList(std::move(snake_clone_list), std::move(apple_clone_list));

	return true;
}

}
}
