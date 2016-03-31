#ifndef ACOROSS_ASIO_PROTOBUF_RPC_STUB_H_
#define ACOROSS_ASIO_PROTOBUF_RPC_STUB_H_

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <memory>
#include <deque>
#include <unordered_map>
#include <functional>
#include <atomic>

#include <google/protobuf/message.h>
#include "rpc_packet.h"

namespace acoross {
namespace rpc {

using boost::asio::ip::tcp;

class RpcPacket;

class RpcStub 
	: public std::enable_shared_from_this<RpcStub>
{
public:
													/*err_code*/
	RpcStub(boost::asio::io_service& io_service)
		: io_service_(io_service)
		, socket_(io_service)
	{}

	virtual ~RpcStub()
	{}

	void start()
	{
		do_read_header();
	}

	bool Connect(char* host, char* port);
	
protected:
	template<typename ReplyMsgT>
	void RpcCaller(unsigned short msg_type, const ::google::protobuf::Message& rq, std::function<void(ErrCode, ReplyMsgT&)> cb);

private:
	using ReplyCallbackF = std::function<void(ErrCode, RpcPacket&)>;

	void AsyncInvoke(unsigned short msg_type, const ::google::protobuf::Message& rq, ReplyCallbackF&& cb);
	
	void end()
	{
		_ASSERT(0);
	}

	void send(std::shared_ptr<RpcPacket> new_msg);
	void do_write();
	void do_read_header();
	void do_read_body();
	size_t RegisterReplyCallback(ReplyCallbackF&& cb);
	bool process_reply(std::shared_ptr<RpcPacket> msg);

private:
	::boost::asio::io_service& io_service_;
	tcp::socket socket_;	//rpc 전용 소켓

	std::shared_ptr<RpcPacket> read_msg_;
	std::deque<std::shared_ptr<RpcPacket>> write_msgs_;

	std::atomic<size_t> rpc_message_uid_{ 0 };
	std::unordered_map<size_t, ReplyCallbackF> wait_reply_queue_;
};

template<typename ReplyMsgT>
inline void RpcStub::RpcCaller(unsigned short msg_type, const::google::protobuf::Message& rq, std::function<void(ErrCode, ReplyMsgT&)> cb)
{
	AsyncInvoke(0, rq,
		[msg_type_rp = msg_type, cb](ErrCode err_code, RpcPacket& reply_rpc_msg)
	{
		auto msg_type_rq = reply_rpc_msg.get_header().message_type_;
		if (msg_type_rp != msg_type_rq)
		{
			return;
		}

		ReplyMsgT reply_msg;
		reply_msg.ParseFromArray(reply_rpc_msg.body(), reply_rpc_msg.body_length());
		cb(err_code, reply_msg);
	});
}

} //rpc
} //acoross
#endif //ACOROSS_ASIO_PROTOBUF_RPC_STUB_H_