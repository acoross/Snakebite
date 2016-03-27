#ifndef SNAKEBITE_HANDLE_H_
#define SNAKEBITE_HANDLE_H_

namespace acoross {
namespace snakebite {

template <typename T>
class Handle final
{
public:
	using Type = uintptr_t;

	Handle()
		: handle(0)
	{}
	Handle(const T& v)
		: handle(reinterpret_cast<uintptr_t>(&v))
	{}
	Handle(T* ptr)
		: handle(reinterpret_cast<uintptr_t>(ptr))
	{}
	Handle(Type handle)
		: Handle::handle(handle)
	{}

	Type handle;
};

}
}

#endif //SNAKEBITE_HANDLE_H_