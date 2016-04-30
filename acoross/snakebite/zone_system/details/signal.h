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

/////////////////////////////////////
template <typename F>
class EventRelayer;

template <typename F>
class Event
{
public:
	using MySigT = boost::signals2::signal<F>;

	explicit Event()
		: sig_(std::make_shared<MySigT>())
	{}
	explicit Event(std::shared_ptr<MySigT> sig)
		: sig_(sig)
	{}
	explicit Event(Event&& other)
		: sig_(std::move(other.sig_))
	{}

	virtual ~Event()
	{}

	template <typename ObsT>
	boost::signals2::connection connect(const ObsT& observer)
	{
		return sig_->connect(observer);
	}

	template <typename ObsT>
	acoross::auto_connection auto_connect(const ObsT& observer)
	{
		return acoross::make_auto_con(sig_->connect(observer));
	}

	template <typename... Args>
	void invoke(Args&&... args)
	{
		(*sig_)(std::forward<Args>(args)...);
	}

	std::unique_ptr<EventRelayer<F>> make_relayer_up();

protected:
	std::shared_ptr<MySigT> sig_;	// 불변. 단, EventRelayer 의 경우 lifetime 이 owner 인 EventRelayer 보다 길 수 있음.
};

template <typename F>
class EventRelayer final
	: public Event<F>
	, public std::enable_shared_from_this<EventRelayer<F>>
{
public:
	//EventRelayer(EventRelayer&) = delete;	// this class is template... so it uses universal reference... you cannot block lvalue ref ctor only.
	//EventRelayer& operator=(EventRelayer&) = delete;

	explicit EventRelayer(std::shared_ptr<MySigT> sig, auto_connection&& conn_to_event)
		: Event(sig), conn_to_event_(std::move(conn_to_event))
	{}
	explicit EventRelayer(EventRelayer&& other)
		: conn_to_event_(std::forward<acoross::auto_connection>(other.conn_to_event_))
		, Event(std::forward<Event>(other))
	{}
	virtual ~EventRelayer()
	{}

	/*EventRelayer& operator=(EventRelayer&& other)
	{
		sig_ = std::forward(other.sig_);
		conn_to_event_ = std::forward(other.conn_to_event_);
	}*/

private:
	acoross::auto_connection conn_to_event_;	// 불변
};

template<typename F>
inline std::unique_ptr<EventRelayer<F>> Event<F>::make_relayer_up()
{
	auto relay_sig = std::make_shared<MySigT>();
	auto new_conn = this->auto_connect(
		[relay_sig](auto... args)
	{
		(*relay_sig)(args...);
	}
	);

	return std::make_unique<EventRelayer<F>>(relay_sig, std::move(new_conn));
}

}
#endif //ACOROSS_EVENT_H_