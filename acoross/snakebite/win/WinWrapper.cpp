#include "WinWrapper.h"
#include "Resource.h"

namespace acoross {
namespace Win {

Window::Window(HINSTANCE hInstance)
	: hInst_(hInstance)
{
	// 전역 문자열을 초기화합니다.
	::LoadStringW(hInst_, IDS_APP_TITLE, szTitle_, MAX_LOADSTRING);
	::LoadStringW(hInst_, IDC_SNAKEBITEM1, szWindowClass_, MAX_LOADSTRING);
}

//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM Window::MyRegisterClass(WndProcFunc* func)
{
	wcex_.cbSize = sizeof(WNDCLASSEX);

	wcex_.style = CS_HREDRAW | CS_VREDRAW;
	wcex_.lpfnWndProc = func;
	wcex_.cbClsExtra = 0;
	wcex_.cbWndExtra = 0;
	wcex_.hInstance = hInst_;
	wcex_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(IDI_SNAKEBITEM1));
	wcex_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex_.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex_.lpszMenuName = MAKEINTRESOURCEW(IDC_SNAKEBITEM1);
	wcex_.lpszClassName = szWindowClass_;
	wcex_.hIconSm = LoadIcon(wcex_.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassExW(&wcex_);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL Window::InitInstance(int nCmdShow)
{
	HWND hWnd = CreateWindowW(szWindowClass_, szTitle_, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst_, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	::ShowWindow(hWnd, nCmdShow);
	::UpdateWindow(hWnd);

	return TRUE;
}

// 기본 메시지 루프입니다.
int Window::PeekMessegeLoop(LoopFunc func)
{
	HACCEL hAccelTable = LoadAccelerators(hInst_, MAKEINTRESOURCE(IDC_SNAKEBITEM1));
	MSG msg;

	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		func(msg);
	}

	return (int)msg.wParam;
}

}
}
