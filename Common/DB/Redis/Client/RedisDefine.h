#pragma once
#include<string>
#include<vector>
#include"RedisAny.h"
#include"Lua/Engine/Define.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Proto/Message/IProto.h"

#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace redis
{
	enum class Type
	{
		REDIS_NONE,
		REDIS_STRING,
		REDIS_ERROR,
		REDIS_NUMBER,
		REDIS_ARRAY,
		REDIS_BIN_STRING,

	};

	namespace Flag
	{
		constexpr char String = '+';
		constexpr char Error = '-';
		constexpr char Number = ':';
		constexpr char Array = '*';
		constexpr char BinString = '$';
	}
}

namespace redis
{
	class Request final : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<Request>
#endif
	{
	public:
		Request() : mTaskId(0){ }
	public:
		void Clear() final;
		std::string ToString() final;
		int OnSendMessage(std::ostream& os) final;
	public:
		template<typename ... Args>
		static std::unique_ptr<Request> Make(const std::string& cmd, Args&& ... args);

		static std::unique_ptr<Request>
		MakeLua(const std::string& key, const std::string& func, const std::string& json);

		template<typename ... Args>
		static void InitParameter(Request* self, Args&& ... args);
	public:
		inline void SetRpcId(int id) {this->mTaskId = id; }
		inline int GetRpcId() const { return this->mTaskId; }
		inline const std::string& GetCommand() const{ return this->mCommand; }
		inline void SetCommand(const std::string& cmd) { this->mCommand = cmd;}
		inline void AddParameter(const std::string& value) { this->mParameters.emplace_back(value); }
		inline void AddString(const char* str, size_t size) { this->mParameters.emplace_back(str, size); }
		inline void AddParameter(int value) { this->mParameters.emplace_back(std::to_string(value)); }
		inline void AddParameter(long long value) { this->mParameters.emplace_back(std::to_string(value)); }
	private:
		static void Encode(Request* self) { }
		template<typename T, typename... Args>
		static void Encode(Request* self, const T& t, Args... args)
		{
			self->AddParameter(t);
			Request::Encode(self, std::forward<Args>(args)...);
		}

	private:
		int mTaskId;
		std::string mCommand;
		std::list<std::string> mParameters;
	};

	template<typename ... Args>
	std::unique_ptr<Request> Request::Make(const std::string& cmd, Args&& ... args)
	{
		std::unique_ptr<Request> request = std::make_unique<Request>();
		{
			request->SetCommand(cmd);
			Request::InitParameter(request.get(), std::forward<Args>(args)...);
		}
		return request;
	}

	template<typename ... Args>
	void Request::InitParameter(Request* self, Args&& ...args)
	{
		Request::Encode(self, std::forward<Args>(args)...);
	}

	class Response final : public ILuaWrite
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<Response>
#endif
	{
	public:
		Response();
		~Response() final { this->Clear(); }

	public:
		bool IsOk();
		void Clear();
		int WriteToLua(lua_State* lua) const final;
	private:
		int OnDecodeArray(std::istream& os, size_t);
		int OnReceiveFirstLine(std::istream& os, size_t);
	public:
		std::string ToString();
		void SetError(const std::string& err);
		int OnRecvLine(std::istream& os, size_t size);
		int OnRecvMessage(std::istream& os, size_t size);
		bool GetArray(std::vector<const String *> & list);
		bool IsString() const { return this->mType == Type::REDIS_STRING || this->mType == Type::REDIS_BIN_STRING; }
	public:
		bool HasError();
		template<typename T>
		const T* Cast(size_t index);
		const Any* Get(size_t index);
		bool GetValue(size_t index, std::string & value);
		inline Type GetType() const { return this->mType; }
		long long GetNumber() const { return this->mNumber; }
		inline size_t GetArraySize() { return this->mArray.size(); }
		const std::string& GetString() const { return this->mString; }
	private:
		Type mType;
		int mIgnore;
		int mDataSize;
		int mDataCount;
		long long mNumber;
		std::string mString;
		std::vector<Any*> mArray;
	};

	template<typename T>
	const T* Response::Cast(size_t index)
	{
		const Any* any = this->Get(index);
		return dynamic_cast<const T*>(any);
	}
}

namespace acs
{
	class RedisTask final : public acs::IRpcTask<redis::Response>, protected WaitTaskSourceBase
    {
    public:
        explicit RedisTask(int id);
    public:
		inline std::unique_ptr<redis::Response> Await();
		inline void OnResponse(std::unique_ptr<redis::Response> response) final;
    private:
		std::unique_ptr<redis::Response> mMessage;
    };

	inline std::unique_ptr<redis::Response> RedisTask::Await()
	{
		this->YieldTask();
		return std::move(this->mMessage);
	}

	inline void RedisTask::OnResponse(std::unique_ptr<redis::Response> response)
	{
		this->mMessage = std::move(response);
		this->ResumeTask();
	}

    class LuaRedisTask : public acs::IRpcTask<redis::Response>
    {
    public:
        LuaRedisTask(lua_State * lua, int id);
        ~LuaRedisTask() final;
    public:
        int Await();
        void OnResponse(std::unique_ptr<redis::Response> response) final;
    private:
        int mRef;
        lua_State * mLua;
    };
}

