#include <iostream>
#include <memory>
#include <Windows.h>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/snakebite/game_session.h>

int main(int argc, char* argv[])
{
	using namespace acoross::snakebite;

	unsigned int init_snake_count = 110;
	unsigned int init_apple_count = 20;

	if (argc > 1)
	{
		init_snake_count = (unsigned int)_strtoi64(argv[1], NULL, 0);
		std::cout <<"count: " << init_snake_count << std::endl;
	}
	if (argc > 2)
	{
		init_apple_count = (unsigned int)_strtoi64(argv[2], NULL, 0);
		std::cout << "count: " << init_apple_count << std::endl;
	}
	
	std::unique_ptr<GameSession> gamesession = std::make_unique<GameSession>(init_apple_count, 500, 500);

	for (unsigned int i = 0; i < init_snake_count; ++i)
	{
		gamesession->AddSnakeNpc();
	}

	try
	{
		while (true)
		{
			const DWORD frametickdiff = 33;

			static DWORD lasttick = ::GetTickCount();
			DWORD tick = ::GetTickCount();
			auto difftick = (int64_t)tick - lasttick;
			//for (;difftick > frametickdiff; difftick -= frametickdiff)
			if (difftick > frametickdiff)
			{
				static double mean_diff_tick_move = 0;
				gamesession->UpdateMove(frametickdiff);
				DWORD tick2 = ::GetTickCount();
				mean_diff_tick_move += (tick2 - tick);

				gamesession->ProcessCollisions();
				lasttick = tick;

				static double mean_diff_tick = 0;
				mean_diff_tick += (::GetTickCount() - tick2);

				static int cnt = 1;
				++cnt;
				if (cnt % 10 == 0)
				{
					double tmp = mean_diff_tick_move / cnt;
					double tmp2 = mean_diff_tick / cnt;
					auto real_snake_count = gamesession->CalculateSnakeCount();
					auto apple_count = gamesession->CalculateAppleCount();
					printf("loop: %d, snake: %zu, apples: %zu, tick: %.4f, tick2: %.4f\n", cnt, real_snake_count, apple_count, tmp, tmp2);
				}
			}
		}
	}
	catch (std::exception& e)
	{
		std::cout << "what: " << e.what() << std::endl;
	}

	return 0;
}