#include <acoross/snakebite/win/Resource.h>
#include <acoross/snakebite/win/WinWrapper.h>
#include <Windowsx.h>

#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <thread>
#include <string>
//#include <sstream>
#include <atomic>

#define ACOROSS_USE_TBB
#ifdef ACOROSS_USE_TBB
#include "tbb/tbbmalloc_proxy.h"
#endif

#include "local_game_client.h"
#include "game_server.h"

using namespace acoross::snakebite;

std::unique_ptr<LocalGameClient> g_game_client;
std::weak_ptr<GameServer> g_game_server_wp;

//<test>
std::atomic<double> mean_draw_time_ms{ 0 };
//</test>

int last_mouse_pos_x = -1;
int last_mouse_pos_y = -1;

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) // static
{
	switch (message)
	{
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// 메뉴 선택을 구문 분석합니다.
			switch (wmId)
			{
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_KEYDOWN:
		{
			if (wParam == VK_LEFT)
			{
				g_game_client->SetKeyDown(PK_LEFT);
			}
			else if (wParam == VK_RIGHT)
			{
				g_game_client->SetKeyDown(PK_RIGHT);
			}
			else if (wParam == VK_DOWN)
			{
				if (auto server = g_game_server_wp.lock())
				{
					server->RequestToSessionNpcController(
						[](SnakeNpcControlManager& npc_controller)
					{
						npc_controller.AsyncAddSnakeNpc();
					});
				}
			}
			else if (wParam == VK_BACK)
			{
				if (auto server = g_game_server_wp.lock())
				{
					server->RequestToSessionNpcController(
						[](SnakeNpcControlManager& npc_controller)
					{
						npc_controller.AsyncRemoveFirstSnakeNpc();
					});
				}
			}
			else if (wParam == VK_UP)
			{
				if (auto server = g_game_server_wp.lock())
				{
					server->RequestToSession(
						[](GameSession& session)
					{
						session.RequestMakeNewApple();
					});
				}
			}
			else if (wParam == VK_F5)
			{
				g_game_client->InitPlayer();
			}
			else if (wParam == 'G')
			{
				g_game_client->SetGridOnOff();
			}
			else if (wParam == 'F')
			{
				g_game_client->FollowPlayerOnOff();
			}
			else if (wParam == 'Q')
			{
				g_game_client->SetScreenCenterToPlayerPos();
			}
			else if (wParam == 'O')
			{
				::InvalidateRect(hWnd, nullptr, true);
				g_game_client->SetScreenOnOff();
			}
			else if (wParam == 'M')	// 'M' or 'm'
			{
				if (auto server = g_game_server_wp.lock())
				{
					server->RequestToSession(
						[](GameSession& session)
					{
#ifdef _DEBUG
						for (int i = 0; i < 200; ++i)
#else
						for (int i = 0; i < 100; ++i)
#endif
						{
							session.RequestMakeNewApple();
						}
				});

					server->RequestToSessionNpcController(
						[](SnakeNpcControlManager& npc_controller)
					{
#ifdef _DEBUG
						for (int i = 0; i < 100; ++i)
#else
						for (int i = 0; i < 100; ++i)
#endif
						{
							npc_controller.AsyncAddSnakeNpc();
						}
			});
		}
	}
}
		break;
	case WM_KEYUP:
		{
			if (wParam == VK_LEFT)
			{
				g_game_client->SetKeyUp(PK_LEFT);
			}
			else if (wParam == VK_RIGHT)
			{
				g_game_client->SetKeyUp(PK_RIGHT);
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			if (g_game_client)
			{
				short hword = HIWORD(wParam);
				short lword = LOWORD(wParam);
				if (lword == 0)
				{
					g_game_client->FetchAddScalePcnt(hword > 0 ? 2 : -2);
					::InvalidateRect(hWnd, nullptr, true);
				}
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (last_mouse_pos_x == -1 || last_mouse_pos_y == -1)
				break;

			if (wParam & MK_LBUTTON)
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);
				g_game_client->FetchMoveScreen(last_mouse_pos_x - x, last_mouse_pos_y - y);

				last_mouse_pos_x = x;
				last_mouse_pos_y = y;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			last_mouse_pos_x = GET_X_LPARAM(lParam);
			last_mouse_pos_y = GET_Y_LPARAM(lParam);
		}
		break;
	case WM_LBUTTONUP:
		{
			::InvalidateRect(hWnd, nullptr, true);
		}
		break;
	case WM_PAINT:
		{
			RECT client_rect;
			::GetClientRect(hWnd, &client_rect);

			PAINTSTRUCT ps;
			acoross::Win::WDC wdc(::BeginPaint(hWnd, &ps));

			// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
			{
				MeanProcessTimeChecker mean_draw(mean_draw_time_ms);
				g_game_client->Draw(wdc, client_rect);
			}

			// performance monitor
			if (auto game_server = g_game_server_wp.lock())
			{
				RECT rect{
					10,
					10,
					250,
					180
				};

				game_server->RequestToSession(
					[client = g_game_client.get()](GameSession& session)
				{
					client->apple_count_.store(session.CalculateAppleCount());
					client->snake_count_.store(session.CalculateSnakeCount());
				});

				wchar_t outBuf[1000] = { 0, };
				::StringCchPrintfW(
					outBuf, 1000,
					L"snakes: %d, apples: %d, \n"
					L"mean move time: %.4f(ms), \n"
					//L"mean collision time: %.4f(ms)\n"
					L"total zone update time: %.4f(ms)\n"
					L"total zone broadcast time: %.4f(ms)\n"
					L"mean clone time: %.4f(ms)\n"
					L"mean total draw time: %.4f(ms)\n"
					L"mean real draw time: %.4f(ms)\n"
					L"mean frame tick: %.4f(ms)\n"
					L"rpc socket pending write %u\n"
					,
					g_game_client->snake_count_.load(), g_game_client->apple_count_.load(),
					game_server->mean_move_time_ms_.load(),
					//game_server->mean_collision_time_ms_.load(),
					game_server->GetUpdateTime(),
					game_server->GetBroadcastTime(),
					game_server->mean_clone_object_time_ms_.load(),
					mean_draw_time_ms.load(),
					g_game_client->mean_draw_time_ms_.load(),
					game_server->mean_frame_tick_.load(),
					acoross::rpc::RpcSocket::pending_write().load()
				);

				size_t str_len = 0;
				::StringCchLengthW(outBuf, 1000, &str_len);

				wdc.DrawTextW(std::wstring(outBuf), rect, DT_CENTER);
				//::DrawTextW(wdc.Get(), outBuf, (int)str_len, &rect, DT_CENTER);
			}

			::EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
}
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.
	boost::asio::io_service io_service;
	auto server = std::make_shared<GameServer>(
		io_service, 22000, 22001);

	g_game_client = std::make_unique<LocalGameClient>(*server);
	g_game_server_wp = server;

	auto auto_con = server->MakeConnectionToGlobalUpdateEvent(
		[client = g_game_client.get()](SbGeoZone::UpdateEventData ed)
	{
		client->SetObjectList_FilteredByCurrentObservingZoneOnly(ed);
	});

	std::thread game_threads[3];
	for (auto& game_thread : game_threads)
	{
		game_thread = std::thread(
			[&io_service]()
		{
			try
			{
				io_service.run();
			}
			catch (std::exception& exp)
			{
				::MessageBoxA(nullptr, exp.what(), "shit", MB_OK);
			}
		});
	}

	// 응용 프로그램 초기화를 수행합니다.
	acoross::Win::Window window(hInstance);
	window.MyRegisterClass(WndProc);
	if (!window.InitInstance(nCmdShow))
	{
		return FALSE;
	}

	auto loop = [](MSG& msg)
	{
		static DWORD lasttick2draw = ::GetTickCount();
		DWORD tick = ::GetTickCount();
		auto difftick2draw = (int64_t)tick - lasttick2draw;
		if (difftick2draw > 33)
		{
			InvalidateRect(msg.hwnd, nullptr, false);
			lasttick2draw = tick;
		}
	};
	window.PeekMessegeLoop(loop);

	io_service.stop();
	for (auto& game_thread : game_threads)
	{
		game_thread.join();
	}

	return 0;
}