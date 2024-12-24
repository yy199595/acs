//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_HTTPTASK_H
#define APP_HTTPTASK_H
#include"Http/Common/HttpResponse.h"
#include"Rpc/Async/RpcTaskSource.h"

namespace acs
{
	class HttpRequestTask : public IRpcTask<http::Response>, protected WaitTaskSourceBase
    {
    public:
		~HttpRequestTask() final = default;
		explicit HttpRequestTask() : IRpcTask<http::Response>(0), mData(nullptr) { }
    public:
        inline std::unique_ptr<http::Response>  Await();
		inline void OnResponse(std::unique_ptr<http::Response> response) final;
	private:
		std::unique_ptr<http::Response> mData;
    };

	std::unique_ptr<http::Response> HttpRequestTask::Await()
	{
		this->YieldTask();
		return std::move(this->mData);
	}

	void HttpRequestTask::OnResponse(std::unique_ptr<http::Response> response)
	{
		this->mData = std::move(response);
		this->ResumeTask();
	}

	class HttpCallbackTask : public IRpcTask<http::Response>
	{
	public:
		explicit HttpCallbackTask(std::function<void(std::unique_ptr<http::Response>)> & cb)
			: mCallback(cb), IRpcTask<http::Response>(0) { }
	private:
		inline void OnResponse(std::unique_ptr<http::Response> response) final;
	private:
		std::function<void(std::unique_ptr<http::Response>)> mCallback;
	};

	inline void HttpCallbackTask::OnResponse(std::unique_ptr<http::Response> response)
	{
		this->mCallback(std::move(response));
	}
}

namespace acs
{
    class HttpRequestClient;
    class LuaHttpRequestTask : public IRpcTask<http::Response>
    {
    public:
        explicit LuaHttpRequestTask(lua_State * lua);
        ~LuaHttpRequestTask() final;
    public:     
        int Await();
        void OnResponse(std::unique_ptr<http::Response> response) final;
    private:
        int mRef;
        lua_State * mLua;
    };
}

#endif //APP_HTTPTASK_H
