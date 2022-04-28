#pragma once
#include<list>
#include<string>
#include<vector>
#include"XCode/XCode.h"
#include"Json/JsonWriter.h"
#include"google/protobuf/message.h"
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
	class RedisValue
	{
	public:
		virtual bool IsLong() = 0;
		virtual bool IsString() = 0;
		virtual std::string & GetValue() = 0;
	};

	class RedisStringType : public RedisValue
	{
	public:
		RedisStringType(const std::string & value);
		RedisStringType(const char * str, size_t size);
	public:
		bool IsLong() final { return false; }
		bool IsString() final { return true;}
		std::string & GetValue() final { return this->mValue;}
	private:
		std::string mValue;
	};

	class RedisLongType : public RedisValue
	{
	private:

	};
}

namespace Sentry
{
	class RedisRequest
    {
    public:
        RedisRequest(const std::string & cmd);
    public:
        void GetCommand(std::iostream & readStream) const;

		template<typename ... Args>
		static std::shared_ptr<RedisRequest> Make(const std::string & cmd, Args &&... args);

		static std::shared_ptr<RedisRequest> MakeLua(const std::string & key, const std::string & func,
				std::list<std::string> & keys, std::list<std::string> & values);

        template<typename ... Args>
        void InitParameter(Args &&... args);
    public:
		const std::string ToJson();
        void AddParameter(int value);
        void AddParameter(long long value);
		void AddParameter(const Message & message);
        void AddParameter(const std::string & value);
	private:
        void Encode() {}

        template<typename T, typename... Args>
        inline void Encode(const T &t, Args... args)
        {
            this->AddParameter(t);
            this->Encode(std::forward<Args>(args)...);
        }
    private:
        std::string mCommand;
        std::list<std::string> mParameters;
    };

	template<typename ... Args>
	std::shared_ptr<RedisRequest> RedisRequest::Make(const std::string& cmd, Args&& ... args)
	{
		std::shared_ptr<RedisRequest> request
				= std::make_shared<RedisRequest>(cmd);
		request->InitParameter(std::forward<Args>(args)...);
		return request;
	}

    template<typename ... Args>
    void RedisRequest::InitParameter(Args &&...args)
    {
        this->Encode(std::forward<Args>(args)...);
    }

    class RedisResponse
    {
    public:
        bool IsOk();
        RedisResponse();
        void AddValue(long long value);
        void AddValue(RedisRespType type);
        void AddValue(const std::string & data);
        void AddValue(const char * str, size_t size);
	 public:
		long long GetNumber() { return this->mNumber; }
		RedisRespType GetType() { return this->mType; }
		size_t GetArraySize() { return this->mArray.size();}
		bool GetString(std::string & value, size_t index = 0);
		bool HasError() { return this->mType == RedisRespType::REDIS_ERROR;}
    private:
        long long mNumber;
        RedisRespType mType;
        std::vector<std::string> mArray;
    };

}

