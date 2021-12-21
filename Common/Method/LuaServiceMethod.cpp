#include"LuaServiceMethod.h"
#include"Script/LuaInclude.h"
#include"Core/App.h"
#include"Rpc/RpcClientComponent.h"
#include"Scene/LuaScriptComponent.h"

namespace GameKeeper
{

	LuaServiceMethod::LuaServiceMethod(const std::string & name, lua_State * lua, int idx)
		:ServiceMethod(name), mLuaEnv(lua), mIdx(idx)
	{
		this->mScriptComponent = App::Get().GetComponent<LuaScriptComponent>();
		this->mRpcClientComponent = App::Get().GetComponent<RpcClientComponent>();
	}

	XCode LuaServiceMethod::Invoke(const com::Rpc_Request &messageData, com::Rpc_Response & response)
	{		

		return XCode::Successful;
	}

	int LuaServiceMethod::Response(lua_State * lua)
	{
//		XCode code = (XCode)lua_tointeger(lua, 2);
//        com::Rpc_Request * responseData = (com::Rpc_Request*)lua_touserdata(lua, 1);
//
//		TcpNetProxyComponent * sessionComponent = App::Get().GetComponent<TcpNetProxyComponent>();
//		responseData->ClearMessage();
//		if (lua_isstring(lua, 3))
//		{
//			size_t size = 0;
//			const char * json = lua_tolstring(lua, 3, &size);
//			const std::string & responseName = responseData->GetProConfig()->Response;
//			Message * message = MessagePool::NewByJson(responseName, json, size);
//			if (message != nullptr)
//			{
//				GKAssertBreakFatal_F(message);
//				GKAssertBreakFatal_F(responseData->SetMessage(message));
//			}
//			else
//			{
//				responseData->SetMessage(json, size);
//			}
//		}
//		sessionComponent->SendNetMessage(responseData);

		return 0;
	}
}
