#include"WatchDogComponent.h"
#include"Config/ServerConfig.h"
#include"Component/NodeMgrComponent.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
	WatchDogComponent::WatchDogComponent()
	{
		this->mInnerComponent = nullptr;
	}

	void WatchDogComponent::ShowLog(spdlog::level::level_enum lv, const std::string& log)
	{		
		s2s::log::show * request = new s2s::log::show();
		{
			request->set_content(log);
			request->set_level((int)lv);
			request->set_name(ServerConfig::Inst()->Name());
		}
		Asio::Context& io = this->mApp->MainThread();
		io.post(std::bind(&WatchDogComponent::ShowLogInWatchDog, this, request));		
	}

	bool WatchDogComponent::LateAwake()
	{
		this->mAddress = fmt::format("127.0.0.1:{0}", 3344);
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}
	
	void WatchDogComponent::ShowLogInWatchDog(s2s::log::show* log)
	{
		this->mLogs.push(log);
		if (this->mInnerComponent != nullptr)
		{
			const std::string func("WatchDog.ShowLog");
			while (!this->mLogs.empty())
			{
				s2s::log::show* message = this->mLogs.front();
				std::shared_ptr<Rpc::Packet> request =
					Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
				{
					request->WriteMessage(message);
					request->GetHead().Add("func", func);
				}
				delete message;
				this->mLogs.pop();
				this->mInnerComponent->Send(this->mAddress, request);
			}
		}
	}
}