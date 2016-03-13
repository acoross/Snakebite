#include <iostream>
#include <acoross/snakebite/win/WinWrapper.h>

#include <boost/asio.hpp>

#include <acoross/snakebite/game_session.h>

int main()
{
	try
	{
		boost::asio::io_service io_service;
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}