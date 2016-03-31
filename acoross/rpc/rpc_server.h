#ifndef ACOROSS_RPC_SERVER_H_
#define ACOROSS_RPC_SERVER_H_

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <functional>

namespace acoross {
namespace rpc {

using boost::asio::ip::tcp;

class RpcServer
{
public:
	using AcceptCallbackF = std::function<void(::boost::asio::io_service&, tcp::socket&&)>;

	RpcServer(boost::asio::io_service& io_service
		, short port
		, std::function<void(::boost::asio::io_service&, tcp::socket&&)> on_accept)
		: io_service_(io_service)
		, acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
		, socket_(io_service)
		, on_accept_(on_accept)
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				if (on_accept_)
				{
					on_accept_(io_service_, std::move(socket_));
				}
			}

			do_accept();
		});
	}

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	AcceptCallbackF on_accept_;
};

}
}
#endif //ACOROSS_RPC_SERVER_H_