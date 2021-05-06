#pragma once
#include<set>
#include<string>
#include<functional>
#include<unordered_map>
namespace SoEasy
{
	class AreaActionTable
	{
	public:
		AreaActionTable(int areaId) : mAreaId(areaId){ }
		~AreaActionTable() { }
	public:
		int GetAreaId() { return mAreaId; }
		bool RemoveAddress(const std::string & address);
		void AddActionAddress(const std::string & name, const std::string & address);
		bool GetActionAddress(const std::string & name, std::vector<std::string> & address);
		void ForeachActions(std::function<bool(const std::string & name, const std::set<std::string> & address)> action);
	private:
		int mAreaId;
		std::unordered_map<std::string, std::set<std::string>> mActionMap;
	};
}