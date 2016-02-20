#ifndef ACOROSS_WINRAPPER_H_
#define ACOROSS_WINRAPPER_H_

#include <Windows.h>

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


// Draw

	void Rectangle(int left, int top, int right, int bottom)
	{
		::Rectangle(hdc_, left, top, right, bottom);
	}

	void Ellipse(int left, int top, int right, int bottom)
	{
		::Ellipse(hdc_, left, top, right, bottom);
	}

protected:
	HDC hdc_;
};

}
}
#endif //ACOROSS_WINRAPPER_H_
