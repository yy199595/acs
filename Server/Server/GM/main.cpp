#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#include<iostream>
#include<asio.hpp>
#include"App/AppLocation.h"

int main(int argc, char ** argv)
{

	std::string address = argc == 2 
		? argv[1] : "127.0.0.1:1020";

	AppLocation app;
	return app.Run(address);
}