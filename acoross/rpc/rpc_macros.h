#ifndef ACOROSS_RPC_MACROS_H_
#define ACOROSS_RPC_MACROS_H_


// DEF_STUB
#define DEF_STUB(name, requestT, replyT)\
	void name(const requestT& rq, std::function<void(rpc::ErrCode, replyT&)> cb)\
	{\
		RpcCaller<replyT>((unsigned short)Protocol::name, rq, std::move(cb));\
	}

// DEF_SERVICE
#define DEF_SERVICE(name, requestT, replyT)\
	virtual rpc::ErrCode name(const requestT& rq, replyT* rp) = 0;

// DEF_SERVICE_IMPL
#define DEF_SERVICE_IMPL(class_name, name, requestT, arg_rq, replyT, arg_rp)\
	rpc::ErrCode class_name::name(const requestT& rq, replyT* rp)

// REGISTER_SERVICE
#define REGISTER_SERVICE(name, requestT, replyT)\
	procedures_[(unsigned short)Protocol::name] = \
		std::make_shared<rpc::ProcedureCaller<requestT, replyT>>(	\
			[this](requestT& rq, replyT* rp)->rpc::ErrCode	\
		{\
			return this->name(rq, rp);	\
		}\
		);


#endif //ACOROSS_RPC_MACROS_H_