#ifndef ACOROSS_RPC_STUB_H_
#define ACOROSS_RPC_STUB_H_

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>

#include <google/protobuf/message.h>
#include "rpc_packet.h"
#include "rpc_socket.h"

namespace acoross {
namespace rpc {

using boost::asio::ip::tcp;

//---------------------------------------------//

class RpcStub 
	: public RpcSocket
{
public:
	RpcStub(boost::asio::io_service& io_service, tcp::socket socket)
		: RpcSocket(io_service, std::move(socket))
	{}

	virtual ~RpcStub()
	{}
		
protected:
	template<typename ReplyMsgT>
	void RpcCaller(unsigned short msg_type, const ::google::protobuf::Message& rq, std::function<void(ErrCode, ReplyMsgT&)> cb);

private:
	using ReplyCallbackF = std::function<void(ErrCode, RpcPacket&)>;

	size_t RegisterReplyCallback(ReplyCallbackF&& cb);
	virtual bool process_msg(RpcPacket& msg) override;

private:
	std::atomic<size_t> rpc_message_uid_{ 0 };
	std::unordered_map<size_t, ReplyCallbackF> wait_reply_queue_;
};

//---------------------------------------------//

template<typename ReplyMsgT>
inline void RpcStub::RpcCaller(unsigned short msg_type, const::google::protobuf::Message& rq, std::function<void(ErrCode, ReplyMsgT&)> cb)
{
	auto self(shared_from_this());
	auto rpc_msg_uid = RegisterReplyCallback(
		[msg_type_rp = msg_type, cb, self](ErrCode err_code, RpcPacket& reply_rpc_msg)
	{
		auto msg_type_rq = reply_rpc_msg.message_type();
		if (msg_type_rp != msg_type_rq)
		{
			return;
		}

		ReplyMsgT reply_msg;
		reply_msg.ParseFromArray(reply_rpc_msg.body(), reply_rpc_msg.body_length());
		cb(err_code, reply_msg);
	});

	auto msg = std::make_shared<RpcPacket>();
	rq.SerializeToArray(msg->body(), msg->max_body_length);
	msg->encode_header(msg_type, rq.ByteSize(), rpc_msg_uid);

	send(msg);
}

} //rpc
} //acoross
#endif //ACOROSS_RPC_STUB_H_