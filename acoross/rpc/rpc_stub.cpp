#include "rpc_stub.h"

namespace acoross {
namespace rpc {

size_t RpcStub::RegisterReplyCallback(ReplyCallbackF&& cb)
{
	auto uid = rpc_message_uid_.fetch_add(1);
	wait_reply_queue_[uid] = cb;

	return uid;
}

/* virtual */
bool RpcStub::process_msg(RpcPacket& msg)
{
	auto rpc_msg_uid = msg.get_header().rpc_msg_uid_;	
	auto err_code = msg.get_header().error_code_;

	auto it = wait_reply_queue_.find(rpc_msg_uid);
	if (it != wait_reply_queue_.end())
	{
		auto callback = std::move(it->second);

		wait_reply_queue_.erase(it);
		callback(err_code, msg);
	}

	return true;
}

}
}