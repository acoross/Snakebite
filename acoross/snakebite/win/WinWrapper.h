#ifndef ACOROSS_WINRAPPER_H_
#define ACOROSS_WINRAPPER_H_

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <windows.h>
#include <strsafe.h>

#ifdef min
#undef min
#endif

#include <functional>
#include <string>

//#include "Resource.h"

#define MAX_LOADSTRING 100

namespace acoross {
namespace Win {

class MemDC;

// wrapper for HDC
class WDC
{
public:
	WDC(HDC hdc) : hdc_(hdc) {}
	WDC(){}
	virtual ~WDC() {}

	HDC Get() const { return hdc_; };

	void Rectangle(int left, int top, int right, int bottom)
	{
		::Rectangle(hdc_, left, top, right, bottom);
	}

	void Ellipse(int left, int top, int right, int bottom)
	{
		::Ellipse(hdc_, left, top, right, bottom);
	}

	void DrawTextW(std::wstring text, RECT& rect, UINT format)
	{
		::DrawTextW(hdc_, text.c_str(), (int)text.length(), &rect, format);
	}

	void DrawTextA(std::string text, RECT& rect, UINT format)
	{
		::DrawTextA(hdc_, text.c_str(), (int)text.length(), &rect, format);
	}

protected:
	HDC hdc_;
};

//////////////////////////////
// class Window

typedef LRESULT(CALLBACK WndProcFunc)(HWND, UINT, WPARAM, LPARAM);
typedef INT_PTR(CALLBACK AboutCallback)(HWND, UINT, WPARAM, LPARAM);

class Window
{
public:
	typedef std::function<void(MSG& msg)> LoopFunc;

	Window(Window&) = delete;
	Window& operator=(Window&) = delete;

public:
	Window(HINSTANCE hInstance);

	//
	//  함수: MyRegisterClass()
	//
	//  목적: 창 클래스를 등록합니다.
	//
	ATOM MyRegisterClass(WndProcFunc* func);

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
	BOOL InitInstance(int nCmdShow);

	// 기본 메시지 루프입니다.
	int PeekMessegeLoop(LoopFunc func);

	// 정보 대화 상자의 메시지 처리기입니다.
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	WCHAR szTitle_[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
	WCHAR szWindowClass_[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
	HINSTANCE hInst_;
	WNDCLASSEXW wcex_;
};

}
}
#endif //ACOROSS_WINRAPPER_H_
