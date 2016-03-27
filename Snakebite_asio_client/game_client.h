#ifndef SNAKEBITE_GAME_CLIENT_H_
#define SNAKEBITE_GAME_CLIENT_H_

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>

#include <acoross/snakebite/protos/snakebite_message.h>
#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>
#include <acoross/snakebite/protos/sc_snakebite_message.pb.h>

#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/snakebite/game_client_base.h>

using boost::asio::ip::tcp;
enum { max_length = 1024 };

namespace acoross {
namespace snakebite {

class GameClient;

class ClientMessageHandlerTable final
{
public:
	bool ProcessMessage(GameClient& client, const SnakebiteMessage& msg);

	bool InitPlayerReply(GameClient& client, sc_messages::InitPlayerSnakeReply& got_msg);

	bool UpdateGameObjectPositions(GameClient& client, sc_messages::UpdateGameObjects& got_msg);
};

class GameClient final
	: public GameClientBase
	, public std::enable_shared_from_this<GameClient>
{
public:
	GameClient(boost::asio::io_service& io_service)
		: io_service_(io_service)
		, socket_(io_service)
	{}

	void ConnectToServer(char* host, char* port)
	{
		tcp::resolver resolver(io_service_);
		boost::asio::connect(socket_, resolver.resolve({ host, port }));

		do_read_header();
	}

	virtual void InitPlayer() override
	{
		SnakebiteMessage msg;
		auto msg_type = SnakebiteMessageType::InitPlayerSnake;

		messages::InitPlayerSnakeRequest rq;
		rq.set_name("remote player");

		rq.SerializeToArray(msg.body(), msg.max_body_length);
		msg.body_length(rq.ByteSize());
		msg.encode_header((unsigned short)msg_type);

		boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
	}

	virtual void SetKeyDown(PlayerKey pk) override
	{
		SnakebiteMessage msg;
		auto msg_type = SnakebiteMessageType::TurnKeyDown;

		messages::TurnKeyDownRequest rq;
		rq.set_key((google::protobuf::int32)pk);

		rq.SerializeToArray(msg.body(), msg.max_body_length);
		msg.body_length(rq.ByteSize());
		msg.encode_header((unsigned short)msg_type);

		boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
	}

	virtual void SetKeyUp(PlayerKey pk) override
	{
		SnakebiteMessage msg;
		auto msg_type = SnakebiteMessageType::TurnKeyUp;

		messages::TurnKeyUpRequest rq;
		rq.set_key((google::protobuf::int32)pk);

		rq.SerializeToArray(msg.body(), msg.max_body_length);
		msg.body_length(rq.ByteSize());
		msg.encode_header((unsigned short)msg_type);

		boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
	}

	virtual void Draw(Win::WDC& wdc, RECT& client_rect) override
	{
		if (clone_list_changed_.load() == false)
		{
			return;
		}

		// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
		// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
		std::list<std::pair<Handle<Snake>::Type, GameObjectClone>> snake_pairs;
		std::list<GameObjectClone> apples;
		RetrieveObjectList(snake_pairs, apples);
		//

		acoross::Win::WDC memdc(::CreateCompatibleDC(wdc.Get()));
		static HBITMAP hbitmap = ::CreateCompatibleBitmap(memdc.Get(), client_rect.right, client_rect.bottom);
		HBITMAP oldbit = (HBITMAP)::SelectObject(memdc.Get(), hbitmap);

		double ratio = 1.0;

		// 테두리 그리기
		memdc.Rectangle(0, 0,
			screen_width, screen_height);

		// TODO
		// 화면과 game_session 크기를 고려해 ratio 를 정한 뒤,
		// ratio 에 따라 크기를 조절해서 그린다.

		{
			MeanProcessTimeChecker mean_draw(mean_draw_time_ms_);
			//auto player = player_.lock();
			for (auto& snake_pair : snake_pairs)
			{
				if (snake_pair.first == player_handle.handle)
				{
					HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(BLACK_BRUSH));
					DrawSnake(memdc, snake_pair.second);
					(HBRUSH)::SelectObject(memdc.Get(), oldbrush);
				}
				else
				{
					DrawSnake(memdc, snake_pair.second);
				}
			}

			for (auto& apple : apples)
			{
				DrawMovingObject(memdc, apple.head_);
			}
		}

		::BitBlt(wdc.Get(), 0, 0, client_rect.right, client_rect.bottom, memdc.Get(), 0, 0, SRCCOPY);

		::SelectObject(memdc.Get(), oldbit);
		::DeleteObject(memdc.Get());
	}

	void do_read_header()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), SnakebiteMessage::header_length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				//end();
			}
		});
	}

	void do_read_body()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec && process_message(read_msg_))
			{
				//room_.deliver(read_msg_);
				do_read_header();
			}
			else
			{
				//end();
			}
		});
	}

	bool process_message(SnakebiteMessage& msg)
	{
		bool ret = message_handler_.ProcessMessage(*this, msg);

		//send(reply);

		return ret;
	}

	void set_player_handle(uintptr_t handle)
	{
		player_handle = handle;
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;

	int screen_width{ 500 };	// init value. set this by server value
	int screen_height{ 500 };	// init value. set this by server value
	Handle<Snake> player_handle{ nullptr };

	SnakebiteMessage read_msg_;
	ClientMessageHandlerTable message_handler_;
};

}
}
#endif //SNAKEBITE_GAME_CLIENT_H_