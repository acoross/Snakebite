#include <iostream>
#include <acoross/snakebite/win/WinWrapper.h>

#include <boost/asio.hpp>

#include "game_client.h"

using boost::asio::ip::tcp;
using acoross::snakebite::GameClient;

int main(int argc, char* argv[])
{
	try
	{
		/*if (argc != 3)
		{
			std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
			return 1;
		}*/

		boost::asio::io_service io_service;

		GameClient game_client(io_service);
		//game_client.ConnectToServer(argv[1], argv[2]);
		game_client.ConnectToServer("localhost", "22000");

		std::cout << "connected to server" << std::endl;

		bool run = true;
		while (run)
		{
			char key = ::getchar();
			std::cout << key << " typed" << std::endl;
			switch (key)
			{
			case 'n':
				{
					game_client.InitPlayer();
				}
				break;
			case 'a':
				{
					
				}
				break;
			case 'q':
				{
					run = false;
				}
				break;
			default:
				break;
			}
		}

		/*char reply[max_length];
		size_t reply_length = boost::asio::read(s,
			boost::asio::buffer(reply, request_length));
		std::cout << "Reply is: ";
		std::cout.write(reply, reply_length);
		std::cout << "\n";*/

		std::cout << "quit" << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}