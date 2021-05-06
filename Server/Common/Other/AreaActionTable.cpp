#include "AreaActionTable.h"

namespace SoEasy
{
	bool AreaActionTable::RemoveAddress(const std::string & address)
	{
		return true;
	}

	void AreaActionTable::AddActionAddress(const std::string & name, const std::string & address)
	{
		auto iter = this->mActionMap.find(name);
		if (iter == this->mActionMap.end())
		{
			std::set<std::string> areaList;
			this->mActionMap.emplace(name, areaList);
		}
		std::set<std::string> & areaList = this->mActionMap[name];
		areaList.insert(address);
	}

	bool AreaActionTable::GetActionAddress(const std::string & name, std::vector<std::string>& addressList)
	{
		auto iter = this->mActionMap.find(name);
		if (iter == this->mActionMap.end())
		{
			return false;
		}
		for (const std::string & address : iter->second)
		{
			addressList.emplace_back(address);
		}
		return true;
	}
	void AreaActionTable::ForeachActions(std::function<bool(const std::string&name, const std::set<std::string> & address)> action)
	{
		for (auto iter = this->mActionMap.begin(); iter != this->mActionMap.end(); iter++)
		{
			const std::string & name = iter->first;
			if (action(name, iter->second) == false)
			{
				break;
			}
		}
	}
}
