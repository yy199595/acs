//
// Created by yjz on 2022/10/24.
//

#include "Request.h"

namespace Tendo
{
	bool HttpRequest::GetHead(const std::string& k, std::string& value) const
	{
		auto iter = this->mHead.find(k);
		if(iter == this->mHead.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}
	bool HttpRequest::AddHead(const std::string& k, int v)
	{
		auto iter = this->mHead.find(k);
		if(iter != this->mHead.end())
		{
			return false;
		}
		this->mHead.emplace(k, std::to_string(v));
		return true;
	}

	bool HttpRequest::AddHead(const std::string& k, const std::string& v)
	{
		auto iter = this->mHead.find(k);
		if(iter != this->mHead.end())
		{
			return false;
		}
		this->mHead.emplace(k, v);
		return true;
	}
}