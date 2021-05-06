#include"AppLocation.h"
#include<iostream>
AppLocation::AppLocation()
{
	this->mIsStop = false;
	this->mIsLoginSuccessful = false;
}

int AppLocation::Run(std::string address)
{
	size_t pos = address.find(":");
	if (pos == std::string::npos)
	{
		return -2;
	}
	const std::string ip = address.substr(0, pos);
	const unsigned short port = std::stoi(address.substr(pos + 1));
	this->mNetWorkClient = new NetWorkClient(mAsioContext, ip, port);

	this->mNetWorkClient->StartConnect();
	return this->MainLoop();
}

void AppLocation::Stop()
{
	this->mIsStop = false;
}

int AppLocation::MainLoop()
{
	std::string message;
	while (this->mIsStop == false)
	{
		this->mAsioContext.poll();
		if (this->mNetWorkClient->TryGetNetMessage(message))
		{
			SoEasy::RapidJsonReader jsonReader;
			if (!jsonReader.TryParse(message))
			{
				continue;
			}
			int code = 1;
			jsonReader.TryGetValue("code", code);
			if (code != 0)
			{
				std::string errorMessage;
				jsonReader.TryGetValue("Error", errorMessage);
#ifdef _WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 4);
				printf("[Error ] %s\n", errorMessage.c_str());
#else
				printf("%s[Error ] %s\e[0m\n", "\e[31m", errorMessage.c_str());
#endif
			}
			else
			{
				std::string gm;
				std::string successfulMessage;
				if (jsonReader.TryGetValue("GM", gm) && gm == "login")
				{
					jsonReader.TryGetValue("list", mGMList);
					this->mIsLoginSuccessful = true;
				}
				jsonReader.TryGetValue("Message", successfulMessage);
#ifdef _WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 2);
				printf("Successful\n%s\n", successfulMessage.c_str());
#else
				printf("%sSuccessful\n%s\e[0m\n", "\e[32m", successfulMessage.c_str());
#endif
			}
			this->WaitInputCommand();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		
	}
	return 0;
}

void AppLocation::WaitInputCommand()
{
	const std::string inputPrompt = this->mIsLoginSuccessful ? "Enter Gm Command" 
		: "Please enter your account and password (Login -account@psssword)";
	char buffer[1024] = { 0 };
	while (true)
	{
#ifdef _WIN32	
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		printf("%s : ", inputPrompt.c_str());
#else
		printf("%s%s \e[34m\n", "\e[1m", inputPrompt.c_str());
#endif
		
		std::cin.getline(buffer, 1024);
		std::string nInputCommand = buffer;
		if (this->mIsLoginSuccessful == false)
		{
			if (nInputCommand.find("login ") == std::string::npos)
			{
				continue;
			}
		}
		size_t pos = nInputCommand.find(" ");	
		std::string gm = nInputCommand.substr(0, pos);		
		if (this->mIsLoginSuccessful && !this->HasGmCommand(gm))
		{
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 4);
			printf("[Error ] Please Enter the correct command\n");
#else
			printf("%s[Error ] Please Enter the correct commandl\e[0m\n", "\e[31m");
#endif
			continue;
		}
		SoEasy::RapidJsonWriter json;
		std::string args = nInputCommand.substr(pos + 1);
		json.AddParameter("GM", gm);
		json.AddParameter("Args", args);
		const std::string & message = json.Serialization();

		this->mNetWorkClient->SendPackage(message);
		break;
	}
}

bool AppLocation::HasGmCommand(const std::string & gm)
{
	for (size_t index = 0; index < this->mGMList.size(); index++)
	{
		if (this->mGMList[index] == gm)
		{
			return true;
		}
	}
	return false;
}
