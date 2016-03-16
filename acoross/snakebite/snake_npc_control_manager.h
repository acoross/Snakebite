#ifndef SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_
#define SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_

#include <memory>
#include <mutex>
#include <random>

#include <acoross/snakebite/snake.h>

namespace acoross {
namespace snakebite {

class GameSession;

class SnakeNpcControlManager final
{
public:
	using SnakeWP = std::weak_ptr<Snake>;
	using MapSnakeWP = std::map<Snake*, std::weak_ptr<Snake>>;

	SnakeNpcControlManager(SnakeNpcControlManager&) = delete;
	SnakeNpcControlManager& operator=(SnakeNpcControlManager&) = delete;

public:
	SnakeNpcControlManager(GameSession& game_session);
	~SnakeNpcControlManager() {}

	void ChangeNpcDirection(int64_t diff_in_ms);

	std::weak_ptr<Snake> AddSnakeNpc();
	bool RemoveSnakeNpc(Snake* snake);

private:
	std::default_random_engine random_engine_;

	std::recursive_mutex snake_npcs_mutex_;
	MapSnakeWP snake_npcs_;

	GameSession& game_session_;
};

}
}
#endif //SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_