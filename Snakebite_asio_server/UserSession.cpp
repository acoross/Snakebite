#include <string>

#include "UserSession.h"
#include "game_server.h"
#include <acoross/snakebite/protos/sc_snakebite_message.pb.h>
#include <acoross/snakebite/protos/snakebite_message_type.h>

namespace acoross {
namespace snakebite {

void UserSession::start()
{
	std::string myid = std::to_string((uintptr_t)this);

	/*game_server_->AddUpdateEventListner(
		myid, 
		[us_wp = std::weak_ptr<UserSession>(shared_from_this())](std::shared_ptr<SnakebiteMessage>& msg)
	{
		if (auto us = us_wp.lock())
		{
			us->send(msg);
		}
	});*/
	
	game_session_->AddUpdateEventListner(
		myid,
		[us_wp = std::weak_ptr<UserSession>(shared_from_this())]
	(const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>& snake_clone_list, const std::list<GameObjectClone>& apple_clone_list)
	{
		if (auto us = us_wp.lock())
		{
			us->send_update_game_object(snake_clone_list, apple_clone_list);
		}
	});

	do_read_header();
}

void UserSession::end()
{
	game_session_->RemoveSnake(user_snake_handle_);

	std::string myid = std::to_string((uintptr_t)this);
	game_session_->UnregisterEventListner(myid);
}


// @atomic

void UserSession::TurnKeyDown(PlayerKey pk)
{
	game_session_->RequestToSnake(user_snake_handle_,
		[pk](Snake& snake)
	{
		snake.SetKeyDown(pk);
	});
}

void UserSession::TurnKeyUp(PlayerKey pk)
{
	game_session_->RequestToSnake(user_snake_handle_,
		[pk](Snake& snake)
	{
		snake.SetKeyUp(pk);
	});
}

Handle<Snake>::Type UserSession::RequestInitPlayer(std::string name)
{
	game_session_->RemoveSnake(user_snake_handle_);
	
	user_snake_handle_ = game_session_->AddSnake(name, Snake::EventHandler());
	return user_snake_handle_;
}

void UserSession::send_update_game_object(
	const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>& snake_clone_list, 
	const std::list<GameObjectClone>& apple_clone_list)
{
	sc_messages::UpdateGameObjects game_objects;
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

		for (auto& apple_clone : apple_clone_list)
		{
			auto* clone = game_objects.add_clone();
			clone->set_clone_type(1);
			clone->set_handle(0);
			clone->set_obj_name(apple_clone.Name);

			auto* head = clone->mutable_head();
			head->set_radius(apple_clone.head_.GetRadius());
			head->set_x(apple_clone.head_.GetPosition().x);
			head->set_y(apple_clone.head_.GetPosition().y);
		}
	}

	if (bool is_initialized = game_objects.IsInitialized())
	{
		auto msg = std::make_shared<SnakebiteMessage>();
		if (bool is_serialize_success = game_objects.SerializeToArray(msg->body(), msg->max_body_length))
		{
			msg->body_length((unsigned short)game_objects.ByteSize());
			msg->encode_header((unsigned short)SC_SnakebiteMessageType::UpdateGameObjects);

			send(msg);
		}
	}
}

}
}
