#include"RpcService.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Lua/LuaServiceMethod.h"
#include"Server/Config/ServiceConfig.h"
#include"Util/String/StringHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Component/InnerRpcComponent.h"
namespace Tendo
{
	bool RpcService::LateAwake()
	{
		return ClusterConfig::Inst()->GetServerName(this->GetName(), this->mCluster);
	}
}
