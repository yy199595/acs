#pragma once
#include<vector>
#include"Core/Component/Component.h"
#include"Proto/Message/ProtoMessage.h"
#include"Telnet/Client/TelnetClientContext.h"

namespace Sentry
{
	class TelnetProto : public Tcp::ProtoMessage
	{
	 public:
		TelnetProto() = default;
		~TelnetProto() = default;
	 public:
		void Add(const std::string & content);
		void Add(const char * str, size_t size);
		int Serialize(std::ostream & os) final; //返回剩余要发送的字节数
	 private:
		std::list<std::string> mContents;
	};
}

namespace Tcp
{
	class TelnetClientContext;
}
namespace Sentry
{
	class ConsoleComponent : public Component, public IComplete
	{
	 public:
		ConsoleComponent() = default;
	 private:
		void Close(const std::string & request);
		void OnClusterComplete() final;
	 private:
		void Update();
	 private:
		class Unit * mCommandUnit;
		class HttpComponent * mHttpComponent;
		class AsyncMgrComponent* mTaskComponent;
	};
}

