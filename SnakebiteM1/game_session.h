#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <memory>
#include <utility>

#include "moving_object.h"
#include "moving_object_container.h"
#include "snake_piece.h"

namespace acoross {
namespace snakebite {

// 맵, MovingObject 로 구성되는 하나의 게임 단위.
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

	// moving_objects_ 의 위치를 갱신한다.
	void UpdateMove(int64_t diff_in_ms);
	
	void ProcessCollisions()
	{
		// TODO: [충돌했을 때, 겹쳐져 있는 상태, 충돌에서 벗어날 때] 를 구분해서 이벤트 발생시킨다.
		// 다른 오브젝트와 충돌했나?
		for (auto& mo1 : container_.GetMovingObjects())
		{
			for (auto& mo2 : container_.GetMovingObjects())
			{
				if (acoross::snakebite::IsCrashed(*mo1, *mo2))
				{
					//DoubleDispatch...
					std::cout << "crashed" << std::endl;
				}
			}
		}
	}

	//임시로 열어주는 API
	ListMovingObject& GetMovingObjects()
	{ 
		return container_.GetMovingObjects();
	}

	// 임시
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