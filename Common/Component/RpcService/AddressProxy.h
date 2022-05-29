//
// Created by yjz on 2022/5/29.
//

#ifndef _ADDRESSPROXY_H_
#define _ADDRESSPROXY_H_
#include<set>
#include<queue>
#include<vector>
#include<string>
#include<unordered_map>
namespace Sentry
{
	class AddressProxy final
	{
	 public:
		bool DelAddress(const std::string & address);
		bool HasAddress(const std::string & address);
		void AddAddress(const std::string & address);
		void AddAddress(std::vector<std::string> & list);
	 public:
		bool DelUserAddress(long long userId);
		bool GetUserAddress(long long userId, std::string & address);
		bool AddUserAddress(long long userId, const std::string & address);
	 public:
		bool GetAddress(std::string & address);
		bool GetAddress(long long id, std::string & address);
		bool GetAllAddress(std::vector<std::string> & list);
		const size_t GetSize() const { return this->mAllAddress.size(); }
	 private:
		std::set<std::string> mAllAddress;
		std::queue<std::string> mAddressQueue;
		std::unordered_map<long long, std::string> mUserAddress;
	};
}

#endif //_ADDRESSPROXY_H_
