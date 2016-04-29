#ifndef ACOROSS_EVENT_H_
#define ACOROSS_EVENT_H_

#include <memory>

#include <boost/signals2.hpp>

namespace acoross {

using ConT = boost::signals2::connection;

namespace {
class disconnector
{
public:
	void operator()(ConT* con)
	{
		con->disconnect();
		delete con;
	}
};
}

using auto_connection = std::unique_ptr<ConT, disconnector>;

inline auto_connection make_auto_con(ConT&& con)
{
	return auto_connection(new ConT(con));
}

//class auto_connection
//{
//public:
//	auto_connection(ConT&) = delete;
//	auto_connection& operator=(auto_connection&) = delete;
//
//	auto_connection()
//		: unique_(nullptr)
//	{}
//
//	auto_connection(ConT&& con)
//		: unique_(new ConT(con))
//	{}
//
//	auto_connection(auto_connection&& acon)
//		: unique_(std::move(acon.unique_))
//	{}
//
//	auto_connection(ConT* con)
//		: unique_(con)
//	{}
//
//	auto_connection& operator=(auto_connection&& acon)
//	{
//		unique_.swap(acon.unique_);
//		return *this;
//	}
//
//	bool operator!() const
//	{
//		return std::atomic_load(&unique_) == nullptr ? true : false;
//	}
//
//	ConT* operator->()
//	{
//		return unique_.get();
//	}
//
//private:
//	class disconnector
//	{
//	public:
//		void operator()(ConT* con)
//		{
//			con->disconnect();
//			delete con;
//		}
//	};
//
//	std::unique_ptr<ConT, auto_connection::disconnector> unique_;
//};
}
#endif //ACOROSS_EVENT_H_