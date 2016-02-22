#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <memory>
#include <utility>

#include "moving_object.h"
#include "moving_object_container.h"

namespace acoross {
namespace snakebite {

class SnakePiece
{
public:
	template<typename... Args>
	SnakePiece(MovingObject& moving_object, const Degree& angle, double velocity, double ang_vel)
		: snake_body_next_(nullptr)
		, moving_object_(moving_object)
		, angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
	{
		;
	}

	void SetAngle(const Degree& angle)
	{
		angle_ = angle;
	}

	void Turn(const Degree& diff)
	{
		angle_ = angle_ + diff;
	}

	Degree GetAngle() const { return angle_; }
	double GetVelocity() const { return velocity_; }
	double GetAngVelocity() const { return ang_vel_; }

	MovingObject& GetMovingObject() { return moving_object_; }

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	std::unique_ptr<SnakePiece> snake_body_next_;
	MovingObject& moving_object_;
};

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