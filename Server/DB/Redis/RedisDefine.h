#pragma once
#include<list>
#include<string>
#include<vector>
#include"XCode/XCode.h"
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
        const std::string & GetValue(size_t index = 0);
		long long GetNumber() { return this->mNumber; }
		RedisRespType GetType() { return this->mType; }
		size_t GetArraySize() { return this->mArray.size();}
        bool HasError() { return this->mType == RedisRespType::REDIS_ERROR;}
    private:
        long long mNumber;
        RedisRespType mType;
        std::vector<std::string> mArray;
    };

	struct RedisConfig
	{
	 public:
		int mCount;
		std::string mIp;
		unsigned short mPort;
		std::string mAddress;
		std::string mPassword;
		std::vector<std::string> mLuaFiles;
	};

}

