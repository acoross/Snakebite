#include "game_client.h"

#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/sc_snakebite_message.pb.h>

namespace acoross {
namespace snakebite {

bool ClientMessageHandlerTable::ProcessMessage(
	GameClient& client, const SnakebiteMessage& msg)
{
	if (msg.message_type() >= static_cast<unsigned short>(SnakebiteMessageType::Max))
	{
		return false;
	}

	bool ret = false;

	auto message_type_typed = static_cast<SC_SnakebiteMessageType>(msg.message_type());
	switch (message_type_typed)
	{
	case SC_SnakebiteMessageType::UpdateGameObjects:
	{
		sc_messages::UpdateGameObjects got_msg;
		got_msg.ParseFromArray(msg.body(), msg.body_length());

		ret = UpdateGameObjectPositions(client, got_msg);
		
		break;
	}
	default:
		break;
	}

	return ret;
}

bool ClientMessageHandlerTable::UpdateGameObjectPositions(
	GameClient& client, sc_messages::UpdateGameObjects& got_msg)
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

		auto client_clone = GameObjectClone(client_head, client_body_list, std::string("noname"));

		if (clone.clone_type() == 0) //snake
		{
			snake_clone_list.emplace_back(std::make_pair(clone.handle(), client_clone));
		}
		else
		{
			apple_clone_list.emplace_back(client_clone);
		}
	}

	client.SetObjectList(std::move(snake_clone_list), std::move(apple_clone_list));

	return true;
}

}
}
