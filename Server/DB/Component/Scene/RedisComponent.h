#pragma once

#include<Util/NumberHelper.h>
#include <Component/Component.h>
#include<RedisClient/RedisTask.h>
#include <Coroutine/CoroutineComponent.h>
#include<QueryResult/InvokeResultData.h>
using namespace GameKeeper;
namespace GameKeeper
{
	class TaskPoolComponent;

	class RedisComponent : public Component
	{
	public:
		RedisComponent();

		~RedisComponent() {}

    public:
        void ClearAllData();
		bool CloseRedisSocket();
		RedisSocket *GetRedisSocket();

	protected:
		bool Awake() final;            //初始化管理器
		void Start() override;                //初始化完成之后
	private:
		redisContext * ConnectRedis(int timeout = 3);
	public:
		bool HasValue(const std::string &key);

		bool HasValue(const std::string &tab, const std::string &key);

	public:
		bool DelValue(const std::string &key);

		bool DelValue(const std::string &tab, const std::string &key);

    public: // hashset
        bool AddToSet(const std::string & set, const std::string &member);

        bool DelFromSet(const std::string & set, const std::string & member);

	public:

        bool SetValue(const std::string & key, const Message & value);

        bool SetValue(const std::string &key, const std::string &value);

        bool SetJsonValue(const std::string & key, const Message & value);

		bool SetValue(const std::string &key, const std::string &value, int second);

        bool SetValue(const std::string &tab, const std::string &key, const Message & value);

        bool SetValue(const std::string &tab, const std::string &key, const std::string &value);

        bool SetJsonValue(const std::string &tba, const std::string &key, const Message & value);

	public:
		bool GetValue(const std::string &key, std::string &value);

        bool GetValue(const std::string &tab, const std::string &key, Message & value);

        bool GetValue(const std::string &tab, const std::string &key, std::string &value);

        bool GetJsonValue(const std::string & key, Message & value);

        bool GetJsonValue(const std::string &tab, const std::string &key, Message & value);

	private:
		std::string mRedisIp;        //redis ip地址
		unsigned short mRedisPort;    //端口号
		TaskPoolComponent *mTaskManager;
		CoroutineComponent *mCorComponent;
		std::unordered_map<std::thread::id, redisContext *> mRedisContextMap;
	};
}