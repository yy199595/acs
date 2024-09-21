//
// Created by yy on 2024/6/23.
//

#ifndef APP_GROUPNOTIFYCOMPONENT_H
#define APP_GROUPNOTIFYCOMPONENT_H
#include <unordered_map>
#include "Entity/Component/Component.h"

namespace notify
{
	struct Data
	{
	public:
		Data(const std::string& k, const std::string& v)
				: key(k), value(v), color("")
		{
		}
		std::string color;
		const std::string key;
		const std::string value;
	};

	struct Base
	{
	public:
		std::string url;
	};

	struct Jump : public Base
	{
		std::string url;
		std::string appid;
		std::string page;
	};

	struct TemplateCard : public Base
	{
		std::string title;
		notify::Jump Jump;
		std::vector<Data> data;
	};

	struct Markdown : public Base
	{
		std::string title;
		std::vector<Data> data;
	};
}

namespace acs
{
	class GroupNotifyComponent : public Component, public ILuaRegister, public IComplete
	{
	public:
		GroupNotifyComponent();
		~GroupNotifyComponent() = default;
	public:
		void SendToWeChat(const std::string & message);
		void SendToDingDing(const std::string & message);
		void SendToWeChat(const notify::Markdown & message);
		void SendToWeChat(const notify::TemplateCard & message, bool async = false);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
	private:
		std::string mWxUrl;
		std::string mDingUrl;
		class HttpComponent * mHttp;
	};
}


#endif //APP_GROUPNOTIFYCOMPONENT_H
