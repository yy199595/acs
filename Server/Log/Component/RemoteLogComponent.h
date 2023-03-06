#pragma once;
#include"Component/Component.h"
namespace Sentry
{
	class RemoteLogComponent final : public Component
	{
	public:
		RemoteLogComponent() = default;
	public:
		bool LateAwake();
	private:
		class RpcService* mRpcService;
	};
}