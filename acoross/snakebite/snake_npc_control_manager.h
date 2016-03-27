#ifndef SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_
#define SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_

#include <memory>
#include <mutex>
#include <random>
#include <map>
#include <set>

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
	SnakeNpcControlManager(std::weak_ptr<GameSession> game_session_wp);
	~SnakeNpcControlManager() {}

	void ChangeNpcDirection(int64_t diff_in_ms);

	Handle<Snake>::Type AddSnakeNpc();
	bool RemoveSnakeNpc(Handle<Snake>::Type handle);
	void RemoveFirstSnakeNpc();

private:
	void UnregisterSnakeNpc(Handle<Snake>::Type handle);

	std::default_random_engine random_engine_;

	std::recursive_mutex snake_npcs_mutex_;
	SetSnakeHandle snake_npc_handles_;

	std::weak_ptr<GameSession> game_session_wp_;
};

}
}
#endif //SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_