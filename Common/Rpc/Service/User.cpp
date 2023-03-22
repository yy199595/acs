//
// Created by zmhy0073 on 2022/10/13.
//

#include"User.h"
#include"Component/InnerNetComponent.h"
#include"Component/NodeMgrComponent.h"
#include"Component/GateComponent.h"
namespace Sentry
{

    bool User::Awake()
    {
        this->mApp->AddComponent<GateComponent>();
        return true;
    }

	bool User::OnInit()
	{
		BIND_COMMON_RPC_METHOD(User::Login);
		BIND_COMMON_RPC_METHOD(User::Logout);
		return true;
	}

    bool User::OnStart()
    {
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
        this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
        return true;
    }

    int User::Login(const s2s::user::login & request)
    {
		long long userId = request.user_id();
		for(int index =0;index<request.list_size();index++)
		{
			const s2s::server::info & info = request.list(index);
			{
				const std::string& server = info.name();
				const std::string& address = info.rpc();
				this->mNodeComponent->AddRpcServer(server, userId, address);
			}
		}
		std::vector<IClient*> clientComponents;
		this->mApp->GetComponents(clientComponents);
		for(IClient * client : clientComponents)
		{
			client->OnLogin(userId);
		}
        return XCode::Successful;
    }

    int User::Logout(const s2s::user::logout &request)
    {
		long long userId = request.user_id();
		this->mNodeComponent->DelUnit(userId);
		std::vector<IClient*> clientComponents;
		this->mApp->GetComponents(clientComponents);
		for(IClient * client : clientComponents)
		{
			client->OnLogout(userId);
		}
		return XCode::Successful;
    }

}