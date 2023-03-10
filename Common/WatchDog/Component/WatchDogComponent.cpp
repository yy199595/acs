#include"WatchDogComponent.h"
#include"Config/ServerConfig.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
	void WatchDogComponent::ShowLog(spdlog::level::level_enum lv, const std::string& log)
	{		
		LogPtr request = std::make_unique<s2s::log::show>();
		{
			request->set_content(log);
			request->set_level((int)lv);
			request->set_name(ServerConfig::Inst()->Name());
		}
		this->mLogs.push(std::move(request));
	}

	bool WatchDogComponent::LateAwake()
	{
		this->mAddress = fmt::format("127.0.0.1:{0}", 3344);
		return true;
	}
	void WatchDogComponent::OnFrameUpdate(float t)
	{
		if (this->mLogs.empty())
		{
			return;
		}
		const std::string func("ShowLog");
		InnerNetComponent* component = this->GetComponent<InnerNetComponent>();
		while (!this->mLogs.empty())
		{
			s2s::log::show* log = this->mLogs.front().get();
			std::shared_ptr<Rpc::Packet> request = 
				Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
			{
				request->WriteMessage(log);
				request->GetHead().Add("func", "WatchDog.ShowLog");
			}
			component->Send(this->mAddress, request);			
			this->mLogs.pop();
		}
	}
}