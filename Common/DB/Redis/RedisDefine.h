#pragma once
#include<list>
#include<string>
#include<vector>
#include"RedisAny.h"
#include"XCode/XCode.h"
#include"Json/JsonWriter.h"
#include"google/protobuf/message.h"
#include"Network/Proto/ProtoMessage.h"
using namespace google::protobuf;

#define REDIS_SAVE_JSON
namespace Sentry
{
	enum class RedisRespType
	{
		REDIS_NONE,
		REDIS_STRING,
		REDIS_ERROR,
		REDIS_NUMBER,
		REDIS_ARRAY,
		REDIS_BIN_STRING,

	};
}

namespace Sentry
{
	class RedisRequest : public Tcp::ProtoMessage
    {
    public:
        RedisRequest(const std::string & cmd);
		~RedisRequest() { }
    public:
		const std::string ToJson() const;
		int Serailize(std::ostream &os) final;
		template<typename ... Args>
		static std::shared_ptr<RedisRequest> Make(const std::string & cmd, Args &&... args);

		static std::shared_ptr<RedisRequest> MakeLua(const std::string & key, const std::string & func, const std::string & json);

        template<typename ... Args>
        static void InitParameter(std::shared_ptr<RedisRequest> self, Args &&... args);
	private:
        void AddParameter(int value);
        void AddParameter(long long value);
		void AddParameter(const Message & message);
        void AddParameter(const std::string & value);
	private:
        static void Encode(std::shared_ptr<RedisRequest> self) {}

        template<typename T, typename... Args>
        static void Encode(std::shared_ptr<RedisRequest> self, const T &t, Args... args)
        {
            self->AddParameter(t);
			RedisRequest::Encode(self, std::forward<Args>(args)...);
        }
    private:
        std::string mCommand;
        std::list<std::string> mParameters;
    };

	template<typename ... Args>
	std::shared_ptr<RedisRequest> RedisRequest::Make(const std::string& cmd, Args&& ... args)
	{
		std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
		RedisRequest::InitParameter(request, std::forward<Args>(args)...);
		return request;
	}

    template<typename ... Args>
    void RedisRequest::InitParameter(std::shared_ptr<RedisRequest> self, Args &&...args)
    {
		RedisRequest::Encode(self, std::forward<Args>(args)...);
    }

    class RedisResponse
    {
	public:
		RedisResponse();
		~RedisResponse();
    public:
        bool IsOk();
		void Clear();
        void AddValue(long long value);
        void AddValue(RedisRespType type);
        void AddValue(const std::string & data);
        void AddValue(const char * str, size_t size);

	 public:
		const RedisAny * Get(size_t index);
		RedisRespType GetType() { return this->mType; }
		size_t GetArraySize() { return this->mArray.size();}
		bool GetString(std::string & value, size_t index = 0);
		bool HasError() { return this->mType == RedisRespType::REDIS_ERROR;}
    private:
        RedisRespType mType;
        std::vector<RedisAny *> mArray;
    };

}

