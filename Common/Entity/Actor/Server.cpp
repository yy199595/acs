//
// Created by leyi on 2023/5/23.
//

#include"Server.h"
#include"XCode/XCode.h"
namespace Tendo
{
	Server::Server(int id, const std::string & name)
		: Actor(id), mName(name), mRpc("Rpc")
	{

	}

	bool Server::OnInit()
	{
		auto iter = this->mListens.find(this->mRpc);
		return iter != this->mListens.end();
	}

	int Server::GetAddress(const std::string& func, std::string& addr)
	{
		auto iter = this->mListens.find(this->mRpc);
		if(iter == this->mListens.end())
		{
			return XCode::AddressAllotFailure;
		}
		addr = iter->second;
		return XCode::Successful;
	}

	void Server::AddListen(const std::string& name, const std::string& addr)
	{
		if (addr.empty() || name.empty())
		{
			return;
		}
		this->mListens[name] = addr;
	}

	bool Server::GetListen(const std::string& name, std::string& addr)
	{
		auto iter = this->mListens.find(name);
		if(iter == this->mListens.end())
		{
			return false;
		}
		addr = iter->second;
		return true;
	}
}