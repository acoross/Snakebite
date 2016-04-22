#ifndef SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_
#define SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_

#include <memory>
#include <mutex>
#include <random>
#include <map>
#include <set>

#include <sdkddkver.h>
#include <boost/asio.hpp>
#include <acoross/snakebite/game_session/game_session_system.h>

namespace acoross {
namespace snakebite {

class GameSession;
class Snake;

class SnakeNpcControlManager final
	: public std::enable_shared_from_this<SnakeNpcControlManager>
{
public:
	using SetSnakeHandle = std::set<Handle<Snake>::Type>;

	SnakeNpcControlManager(SnakeNpcControlManager&) = delete;
	SnakeNpcControlManager& operator=(SnakeNpcControlManager&) = delete;

public:
	SnakeNpcControlManager(::boost::asio::io_service& io_service,
		std::weak_ptr<GameSession> game_session_wp);
	~SnakeNpcControlManager() {}

	void AsyncChangeNpcDirection(int64_t diff_in_ms);
	void AsyncAddSnakeNpc();
	void AsyncRemoveSnakeNpc(Handle<Snake>::Type handle);
	void AsyncRemoveFirstSnakeNpc();

private:
	void async_unregister_snake_npc(Handle<Snake>::Type handle);

	//std::recursive_mutex snake_npcs_mutex_;
	::boost::asio::strand strand_;
	SetSnakeHandle snake_npc_handles_;

	std::default_random_engine random_engine_;
	std::weak_ptr<GameSession> game_session_wp_;
};

}
}
#endif //SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_