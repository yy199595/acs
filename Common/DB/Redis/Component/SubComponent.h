//
// Created by leyi on 2024/1/18.
//

#ifndef APP_SUBCOMPONENT_H
#define APP_SUBCOMPONENT_H
#include"Redis/Client/Client.h"
#include"Yyjson/Document/Document.h"
#include"Redis/Config/RedisConfig.h"
#include"Entity/Component/Component.h"


namespace joke
{
	using SubCallback = std::function<void(const json::r::Document &)>;

	class SubController
	{
	public:
		SubController();
		bool Delete(int id);
		int Add(SubCallback && callback);
		int Count() const { return this->mCallbacks.size(); }
	public:
		int OnInvoke(const std::string & json);
	private:
		int mIndex;
		std::unordered_map<int, SubCallback> mCallbacks;
	};

	class SubComponent : public Component, public IRpc<redis::Request, redis::Response>
	{
	public:
		SubComponent();
		~SubComponent() = default;
	public:
		bool UnSubChannel(const std::string & chane, int id);
		int SubChannel(const std::string & chanel, SubCallback callback);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnConnectOK(int id) final;
		void OnMessage(redis::Request *request, redis::Response *response) final;
	private:
		redis::Config mConfig;
		redis::Client * mClient;
		std::unordered_map<std::string, SubController *> mControllers;
	};
}


#endif //APP_SUBCOMPONENT_H
