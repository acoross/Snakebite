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

	game_server_.AddUpdateEventListner(
		myid, 
		[us_wp = std::weak_ptr<UserSession>(shared_from_this())](GameSession& session)
	{
		if (auto us = us_wp.lock())
		{
			auto snake_list = session.CloneSnakeList();
			auto apple_list = session.CloneAppleList();
			//client->SetObjectList(std::move(snake_list), std::move(apple_list));

			sc_messages::UpdateGameObjects game_objects_;
			
			for (auto& pair : snake_list)
			{
				auto& snake_clone = pair.second;
				auto* clone = game_objects_.add_clone();
				
				clone->set_clone_type(0);
				clone->set_handle(Handle<Snake>(pair.first).handle);

				auto* head = clone->mutable_head();
				head->set_radius(snake_clone.head_.GetRadius());
				head->set_x(snake_clone.head_.GetPosition().x);
				head->set_y(snake_clone.head_.GetPosition().y);
				
				clone->set_body_count((::google::protobuf::int32)snake_clone.body_list_.size());
				for (auto& body : snake_clone.body_list_)
				{
					auto* pac_body = clone->add_body();
					pac_body->set_radius(body.GetRadius());
					pac_body->set_x(body.GetPosition().x);
					pac_body->set_y(body.GetPosition().y);
				}
			}
			
			for (auto& apple_clone : apple_list)
			{
				auto* clone = game_objects_.add_clone();
				clone->set_clone_type(1);
				clone->set_handle(0);

				auto* head = clone->mutable_head();
				head->set_radius(apple_clone.head_.GetRadius());
				head->set_x(apple_clone.head_.GetPosition().x);
				head->set_y(apple_clone.head_.GetPosition().y);
				
				clone->set_body_count(0);
			}

			auto msg = std::make_shared<SnakebiteMessage>();
			game_objects_.SerializeToArray(msg->body(), msg->max_body_length);
			msg->encode_header((unsigned short)SC_SnakebiteMessageType::UpdateGameObjects);

			us->send(msg);
		}
	});

	do_read_header();
}

void UserSession::end()
{
	if (auto my_snake = user_snake_.lock())
	{
		game_session_->RemoveSnake(Handle<Snake>(my_snake.get()).handle);
	}
	
	std::string myid = std::to_string((uintptr_t)this);
	game_server_.UnregisterEventListner(myid);
}

void UserSession::RequestInitPlayer(std::string name)
{
	if (auto player = user_snake_.lock())
	{
		game_session_->RemoveSnake(Handle<Snake>(player.get()).handle);
	}
	user_snake_ = game_session_->AddSnake(Snake::EventHandler(), name);
}

}
}
