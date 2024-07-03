//
// Created by MyPC on 2023/4/15.
//

#ifndef APP_MASTERCOMPONENT_H
#define APP_MASTERCOMPONENT_H
#include"Core/Map/HashMap.h"
#include"Entity/Component/Component.h"
namespace joke
{
	class Server;
	class MasterComponent : public Component, public IComplete
	{
	public:
		MasterComponent();
	public:
		bool SyncServer(long long id = 0);
	private:
		void Complete() final;
		bool LateAwake() final;
		bool RegisterServer() const;
	private:
		std::string mHost;
		std::string mToken;
		class HttpComponent * mHttp;
		class ActorComponent * mActComponent;
	};
}

#endif //APP_MASTERCOMPONENT_H
