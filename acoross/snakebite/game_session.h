#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <memory>
#include <utility>

#include "moving_object_system/moving_object_system.h"
#include "snake_piece.h"

namespace acoross {
namespace snakebite {

// ��, MovingObject �� �����Ǵ� �ϳ��� ���� ����.
class GameSession final
{
public:
	typedef MovingObjectContainer::ListMovingObject ListMovingObject;
	typedef std::list<std::unique_ptr<SnakePiece>> ListSnakePiece;
	
	GameSession() 
	{
		Initialize();
	}
	~GameSession() {}

	void Initialize();
	void CleanUp();

	// moving_objects_ �� ��ġ�� �����Ѵ�.
	void UpdateMove(int64_t diff_in_ms);
	
	void ProcessCollisions()
	{
		container_.GetCollisionContainer().CheckCollisions();
	}

	//�ӽ÷� �����ִ� API
	ListMovingObject& GetMovingObjects()
	{ 
		return container_.GetMovingObjects();
	}

	// �ӽ�
	const MovingObjectContainer& GetContainer() const { return container_; }

private:
	ListSnakePiece snakes_;
	MovingObjectContainer container_;
};

typedef std::shared_ptr<GameSession> GameSessionSP;
typedef std::weak_ptr<GameSession> GameSessionWP;

}
}
#endif //SNAKEBITE_GAME_SESSION_H_