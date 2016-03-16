#ifndef SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_
#define SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_

#include <memory>
#include <mutex>

#include <acoross/snakebite/snake.h>

namespace acoross {
namespace snakebite {

class SnakeNpcControlManager final
{
public:
	using SnakeWP = std::weak_ptr<Snake>;
	using MapSnakeWP = std::map<Snake*, std::weak_ptr<Snake>>;

	SnakeNpcControlManager(SnakeNpcControlManager&) = delete;
	SnakeNpcControlManager& operator=(SnakeNpcControlManager&) = delete;

public:
	SnakeNpcControlManager() {}
	~SnakeNpcControlManager() {}

	void ChangeNpcDirection(int64_t diff_in_ms);

	std::weak_ptr<Snake> AddSnakeNpc();
	bool RemoveSnakeNpc(Snake* snake);

private:
	std::recursive_mutex snake_npcs_mutex_;
	MapSnakeWP snake_npcs_;
};

}
}
#endif //SNAKEBITE_SNAKE_NPC_CONTROL_MANAGER_H_