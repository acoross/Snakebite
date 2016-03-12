#include <iostream>
#include <memory>
#include <Windows.h>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/snakebite/game_session.h>

int main()
{
	using namespace acoross::snakebite;
	std::unique_ptr<GameSession> gamesession = std::make_unique<GameSession>();

	while(true)
	{
		const DWORD frametickdiff = 33;

		static DWORD lasttick = ::GetTickCount();
		DWORD tick = ::GetTickCount();
		auto difftick = (int64_t)tick - lasttick;
		for (;difftick > frametickdiff; difftick -= frametickdiff)
		{
#if defined(_DEBUG)
			gamesession->UpdateMove(frametickdiff);
#else
			gamesession->UpdateMove(difftick);
#endif	
			gamesession->ProcessCollisions();
			lasttick = tick;
		}
	}

	return 0;
}