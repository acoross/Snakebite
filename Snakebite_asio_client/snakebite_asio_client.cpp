#include <acoross/snakebite/win/Resource.h>
#include <acoross/snakebite/win/WinWrapper.h>

#include <iostream>
#include <thread>
#include <memory>
#include <boost/asio.hpp>

#include "game_client.h"

using boost::asio::ip::tcp;
using namespace acoross::snakebite;

std::weak_ptr<GameClient> g_game_client;

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
			if (auto game_client = g_game_client.lock())
			{
				game_client->SetKeyDown(PK_LEFT);
			}
		}
		else if (wParam == VK_RIGHT)
		{
			if (auto game_client = g_game_client.lock())
			{
				game_client->SetKeyDown(PK_RIGHT);
			}
		}
		else if (wParam == VK_F5)
		{
			if (auto game_client = g_game_client.lock())
			{
				game_client->InitPlayer();
			}
		}
	}
	break;
	case WM_KEYUP:
	{
		if (wParam == VK_LEFT)
		{
			if (auto game_client = g_game_client.lock())
			{
				game_client->SetKeyUp(PK_LEFT);
			}
		}
		else if (wParam == VK_RIGHT)
		{
			if (auto game_client = g_game_client.lock())
			{
				game_client->SetKeyUp(PK_RIGHT);
			}
		}
	}
	case WM_PAINT:
	{
		RECT client_rect;
		::GetClientRect(hWnd, &client_rect);

		PAINTSTRUCT ps;
		acoross::Win::WDC wdc(::BeginPaint(hWnd, &ps));

		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
		{
			//MeanProcessTimeChecker mean_draw(mean_draw_time_ms);
			if (auto game_client = g_game_client.lock())
			{
				game_client->Draw(wdc, client_rect);
			}
		}

		/*
		// performance monitor
		if (auto game_server = g_game_server_wp.lock())
		{
			RECT rect{
				10,
				10,
				300,
				150
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
				L"mean collision time: %.4f(ms)\n"
				L"mean clone time: %.4f(ms)\n"
				L"mean server tick time: %.4f(ms)\n"
				L"mean total draw time: %.4f(ms)\n"
				L"mean real draw time: %.4f(ms)\n"
				L"mean frame tick: %.4f(ms)\n"
				,
				g_game_client->snake_count_.load(), g_game_client->apple_count_.load(),
				game_server->mean_move_time_ms_.load(),
				game_server->mean_collision_time_ms_.load(),
				game_server->mean_clone_object_time_ms_.load(),
				game_server->mean_tick_time_ms_.load(),
				mean_draw_time_ms.load(),
				g_game_client->mean_draw_time_ms_.load(),
				game_server->mean_frame_tick_.load()
				);

			size_t str_len = 0;
			::StringCchLengthW(outBuf, 1000, &str_len);

			wdc.DrawTextW(std::wstring(outBuf), rect, DT_CENTER);
			//::DrawTextW(wdc.Get(), outBuf, (int)str_len, &rect, DT_CENTER);
		}
		*/

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

	try
	{
		// TODO: 여기에 코드를 입력합니다.
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::socket socket(io_service);
		{
			tcp::resolver resolver(io_service);
			boost::asio::connect(socket, resolver.resolve({ "127.0.0.1", "22000" }));
		}
		auto game_client = std::make_shared<GameClient>(io_service, std::move(socket));
		g_game_client = game_client;

		std::thread game_thread(
			[&io_service]()
		{
			io_service.run();
		}
		);

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
		game_thread.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
		::MessageBoxA(NULL, e.what(), "error", MB_OK);
	}

	return 0;
}