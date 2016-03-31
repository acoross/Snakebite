#ifndef ACOROSS_RPC_SOCKET_H_
#define ACOROSS_RPC_SOCKET_H_

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <memory>
#include <deque>

#include "rpc_packet.h"

namespace acoross {
namespace rpc {

using boost::asio::ip::tcp;

class RpcSocket
	: public std::enable_shared_from_this<RpcSocket>
{
public:
	RpcSocket(::boost::asio::io_service& io_service, tcp::socket&& socket)
		: io_service_(io_service)
		, socket_(std::move(socket))
	{
	}

	virtual ~RpcSocket(){}

	void start()
	{
		do_read_header();
	}

	void end()
	{
		_ASSERT(0);
	}

	void send(std::shared_ptr<RpcPacket> new_msg);
	void do_write();
	void do_read_header();
	void do_read_body();
	virtual bool process_msg(RpcPacket& msg) = 0;

private:
	::boost::asio::io_service& io_service_;
	tcp::socket socket_;	//rpc 전용 소켓

	RpcPacket read_msg_;
	std::deque<std::shared_ptr<RpcPacket>> write_msgs_;
};

}
}
#endif