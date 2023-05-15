//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_LOCATIONCOMPONENT_H
#define APP_LOCATIONCOMPONENT_H
#include<vector>
#include<string>
#include<unordered_set>
#include<unordered_map>
#include"Entity/Unit/ServerUnit.h"
#include"Entity/Component/Component.h"

namespace Tendo
{
	// 记录所有服务地址
	class LocationComponent final : public Component
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
		bool DelServer(int id);
		void GetAllServer(std::vector<ServerUnit *> & servers);
	public:
		ServerUnit * GetServerById(int id);
		int RangeServer(const std::string & server) const;
		ServerUnit * GetOrCreateServer(int id, const std::string & name);
    public:
		bool HasServer(const std::string & server) const;
		bool GetServerAddress(int id, const std::string & listen, std::string & address);
	private:
		std::unordered_map<int, std::unique_ptr<ServerUnit>> mServers;
		std::unordered_map<std::string, std::vector<int>> mServerNames;
	};
}

#endif //APP_LOCATIONCOMPONENT_H
