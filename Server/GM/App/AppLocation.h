#pragma once
#include<Json/JsonHelper.h>
#include<NetWork/NetWorkClient.h>
class AppLocation
{
public:
	AppLocation();
public:
	int Run(std::string address);
	void Stop();
private:
	int MainLoop();
	void WaitInputCommand();
	bool HasGmCommand(const std::string & gm);
private:
	bool mIsStop;
	bool mIsLoginSuccessful;
	NetWorkClient * mNetWorkClient;
	asio::io_context mAsioContext;
	std::vector<std::string> mGMList;
};