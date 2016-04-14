// Generated by the acoross-rpc protobuf plugin.
// If you make any local change, they will be lost.
// source: snakebite_message.proto
#ifndef ACOROSS_RPC_snakebite_5fmessage_2eproto__INCLUDED
#define ACOROSS_RPC_snakebite_5fmessage_2eproto__INCLUDED

#include "snakebite_message.pb.h"

#include <boost/asio.hpp>
#include <acoross/rpc/rpc_service.h>
#include <acoross/rpc/rpc_stub.h>
#include <acoross/rpc/rpc_macros.h>

namespace acoross {
namespace snakebite {
namespace messages {

<<<<<<< HEAD
class SnakebiteService final {
 public:
  enum Protocol
  {
    InitPlayer,
    SetKeyDown,
    SetKeyUp,
    ListenGameObjectUpdateEvent,
=======
class MyRpc final {
 public:
  enum Protocol
  {
    Hello,
    TurnKeyDown,
    TurnKeyUp,
>>>>>>> origin/master
  };

  class Service : public ::acoross::rpc::RpcService 
  {
  public:
    Service(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket&& socket);
    virtual ~Service() {}

  private:
<<<<<<< HEAD
    DEF_SERVICE(InitPlayer, ::acoross::snakebite::messages::InitPlayerSnakeRequest, ::acoross::snakebite::messages::InitPlayerSnakeReply)
    DEF_SERVICE(SetKeyDown, ::acoross::snakebite::messages::TurnKeyDownRequest, ::acoross::snakebite::messages::VoidReply)
    DEF_SERVICE(SetKeyUp, ::acoross::snakebite::messages::TurnKeyUpRequest, ::acoross::snakebite::messages::VoidReply)
    DEF_SERVICE(ListenGameObjectUpdateEvent, ::acoross::snakebite::messages::ListenGameObjectUpdateRequest, ::acoross::snakebite::messages::UpdateGameObjectsEvent)
=======
    DEF_SERVICE(Hello, ::acoross::snakebite::messages::HelloRequest, ::acoross::snakebite::messages::HelloReply)
    DEF_SERVICE(TurnKeyDown, ::acoross::snakebite::messages::TurnKeyDownRequest, ::acoross::snakebite::messages::VoidReply)
    DEF_SERVICE(TurnKeyUp, ::acoross::snakebite::messages::TurnKeyUpRequest, ::acoross::snakebite::messages::VoidReply)
>>>>>>> origin/master

  };

  class Stub : public ::acoross::rpc::RpcStub
  {
  public:
    Stub(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket&& socket);
    virtual ~Stub() {}
<<<<<<< HEAD
    DEF_STUB(InitPlayer, ::acoross::snakebite::messages::InitPlayerSnakeRequest, ::acoross::snakebite::messages::InitPlayerSnakeReply)
    DEF_STUB(SetKeyDown, ::acoross::snakebite::messages::TurnKeyDownRequest, ::acoross::snakebite::messages::VoidReply)
    DEF_STUB(SetKeyUp, ::acoross::snakebite::messages::TurnKeyUpRequest, ::acoross::snakebite::messages::VoidReply)
    DEF_STUB(ListenGameObjectUpdateEvent, ::acoross::snakebite::messages::ListenGameObjectUpdateRequest, ::acoross::snakebite::messages::UpdateGameObjectsEvent)
=======
    DEF_STUB(Hello, ::acoross::snakebite::messages::HelloRequest, ::acoross::snakebite::messages::HelloReply)
    DEF_STUB(TurnKeyDown, ::acoross::snakebite::messages::TurnKeyDownRequest, ::acoross::snakebite::messages::VoidReply)
    DEF_STUB(TurnKeyUp, ::acoross::snakebite::messages::TurnKeyUpRequest, ::acoross::snakebite::messages::VoidReply)
>>>>>>> origin/master
  };

};


}  // namespace messages
}  // namespace snakebite
}  // namespace acoross


#endif  // ACOROSS_RPC_snakebite_5fmessage_2eproto__INCLUDED