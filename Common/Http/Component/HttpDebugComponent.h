#pragma once;
#include"Client/Message.h"
#include"HttpListenComponent.h"

namespace Sentry
{
	class HttpDebugComponent final : public HttpListenComponent
	{
	public:
		HttpDebugComponent() = default;
	private:
		bool LateAwake() final;
		bool OnDelClient(const std::string& address) final;
		void OnRequest(const std::string& address, std::shared_ptr<Http::Request> request) final;
	private:
		void Invoke(const std::string& address, std::shared_ptr<Rpc::Packet>& message);
		bool GetAddress(const std::string& service, long long id, std::string& address);
	private:
		class NodeMgrComponent* mNodeComponent;
		class InnerNetMessageComponent* mInnerComponent;
	};
}