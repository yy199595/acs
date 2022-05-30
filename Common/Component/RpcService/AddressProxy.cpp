//
// Created by yjz on 2022/5/29.
//

#include "AddressProxy.h"

namespace Sentry
{
	void AddressProxy::AddAddress(const std::string& address)
	{
		auto iter = this->mAllAddress.find(address);
		if(iter == this->mAllAddress.end())
		{
			this->mAllAddress.emplace(address);
			this->mAddressQueue.emplace(address);
		}
	}


	bool AddressProxy::HasAddress(const std::string& address)
	{
		auto iter = this->mAllAddress.find(address);
		return iter != this->mAllAddress.end();
	}

	bool AddressProxy::GetAllAddress(std::vector<std::string>& list)
	{
		list.reserve(this->mAllAddress.size());
		auto iter = this->mAllAddress.begin();
		for(; iter != this->mAllAddress.end(); iter++)
		{
			list.emplace_back(*iter);
		}
		return !list.empty();
	}

	bool AddressProxy::GetAddress(std::string& address)
	{
		if(this->mAddressQueue.empty())
		{
			return false;
		}
		while(!this->mAddressQueue.empty())
		{
			address = this->mAddressQueue.front();
			this->mAddressQueue.pop();
			if(this->HasAddress(address))
			{
				this->mAddressQueue.emplace(address);
				return true;
			}
		}
		return false;
	}

	bool AddressProxy::GetAddress(long long id, std::string& address)
	{
		auto iter = this->mUserAddress.find(id);
		if(iter == this->mUserAddress.end())
		{
			return false;
		}
		address = iter->second;
		return true;
	}

	bool AddressProxy::GetUserAddress(long long userId, std::string& address)
	{
		auto iter = this->mUserAddress.find(userId);
		if(iter != this->mUserAddress.end())
		{
			address = iter->second;
			return true;
		}
		return false;
	}

	bool AddressProxy::DelUserAddress(long long userId)
	{
		auto iter = this->mUserAddress.find(userId);
		if(iter != this->mUserAddress.end())
		{
			this->mUserAddress.erase(iter);
			return true;
		}
		return false;
	}

	bool AddressProxy::AddUserAddress(long long userId, const std::string& address)
	{
		auto iter = this->mUserAddress.find(userId);
		if(iter != this->mUserAddress.end())
		{
			return false;
		}
		this->mUserAddress.emplace(userId, address);
		return true;
	}
	bool AddressProxy::DelAddress(const std::string & address)
	{
		auto iter = this->mAllAddress.find(address);
		if(iter != this->mAllAddress.end())
		{
			this->mAllAddress.erase(iter);
			return true;
		}
		return false;
	}
}