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
		this->mMutex.lock();
		this->mLogs.push(std::move(request));
		this->mMutex.unlock();
	}

	bool WatchDogComponent::LateAwake()
	{
		this->mAddress = fmt::format("127.0.0.1:{0}", 3344);
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}
	void WatchDogComponent::OnFrameUpdate(float t)
	{		
		const std::string func("WatchDog.ShowLog");
		std::lock_guard<std::mutex> lock(this->mMutex);
		while (!this->mLogs.empty())
		{
			s2s::log::show* log = this->mLogs.front().get();
			std::shared_ptr<Rpc::Packet> request = 
				Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
			{
				request->WriteMessage(log);
				request->GetHead().Add("func", func);
			}
			this->mInnerComponent->Send(this->mAddress, request);
			this->mLogs.pop();
		}
	}
}