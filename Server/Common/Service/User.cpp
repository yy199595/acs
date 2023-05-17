//
// Created by zmhy0073 on 2022/10/13.
//

#include"User.h"
#include"Entity/Actor/App.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/LocationComponent.h"
#include"Gate/Component/GateComponent.h"
namespace Tendo
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

    int User::Login(const s2s::user::login & request)
    {
		long long userId = request.user_id();
		for(int index =0;index<request.list_size();index++)
		{
			const s2s::server::info & info = request.list(index);
			{
				//TODO
				const std::string& server = info.server_name();
				//const std::string& address = info.rpc();
				//this->mNodeComponent->AddRpcServer(server, userId, address);
			}
		}
        return XCode::Successful;
    }

    int User::Logout(const s2s::user::logout &request)
    {
		long long userId = request.user_id(); 
		// TODO
		
		return XCode::Successful;
    }

}