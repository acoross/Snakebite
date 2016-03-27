#include "game_server.h"

#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/sc_snakebite_message.pb.h>

void acoross::snakebite::GameServer::do_update_game_session()
{
	MeanProcessTimeChecker mean_tick(mean_tick_time_ms_);
	game_update_timer_.expires_from_now(boost::posix_time::milliseconds(FRAME_TICK));

	static uint64_t last_tick = ::GetTickCount64();
	uint64_t current_tick = ::GetTickCount64();
	uint64_t diff = current_tick - last_tick;

	double new_mean_time = mean_frame_tick_.load() * 0.9 + diff * 0.1;
	mean_frame_tick_.store((double)new_mean_time);

	{
		MeanProcessTimeChecker mean_move(mean_move_time_ms_);
		game_session_->UpdateMove(FRAME_TICK);
	}

	{
		MeanProcessTimeChecker mean_collision(mean_collision_time_ms_);
		game_session_->ProcessCollisions();
	}

	{
		std::lock_guard<std::mutex> lock(update_handler_mutex_);
		MeanProcessTimeChecker mean_clone(mean_clone_object_time_ms_);

		sc_messages::UpdateGameObjects game_objects;
		{
			auto snake_list = game_session_->CloneSnakeList();
			auto apple_list = game_session_->CloneAppleList();

			if (on_update_local_listner_)
			{
				on_update_local_listner_(snake_list, apple_list);
			}

			for (auto& pair : snake_list)
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

			for (auto& apple_clone : apple_list)
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

				for (auto& pair : on_update_event_listeners_)
				{
					auto& listner = pair.second;
					listner(msg);
				}
			}
		}
	}

	last_tick = current_tick;

	game_update_timer_.async_wait(
		[this](boost::system::error_code ec)
	{
		if (!ec)
		{
			do_update_game_session();
		}
	});
}
