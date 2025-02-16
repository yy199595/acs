//
// Created by MyPC on 2023/4/15.
//

#ifndef APP_MASTERCOMPONENT_H
#define APP_MASTERCOMPONENT_H
#include"Entity/Component/Component.h"
namespace acs
{
	class Server;
	class MasterComponent final : public Component, public IComplete
	{
	public:
		MasterComponent();
	public:
		bool SyncServer(long long id = 0);
	private:
		bool LateAwake() final;
		void OnComplete() final;
		bool RegisterServer() const;
	private:
		std::string mHost;
		std::string mToken;
		class HttpComponent * mHttp;
		class ActorComponent * mActComponent;
	};
}

#endif //APP_MASTERCOMPONENT_H
