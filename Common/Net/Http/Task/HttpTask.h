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
		~HttpRequestTask() final { delete this->mData; }
		explicit HttpRequestTask() : IRpcTask<http::Response>(0), mData(nullptr) { }
    public:
        inline http::Response * Await();
		inline void OnResponse(http::Response* response) final;
	private:
		http::Response * mData;
    };

	http::Response* HttpRequestTask::Await()
	{
		this->YieldTask();
		return this->mData;
	}

	void HttpRequestTask::OnResponse(http::Response* response)
	{
		this->mData = response;
		this->ResumeTask();
	}

	class HttpCallbackTask : public IRpcTask<http::Response>
	{
	public:
		explicit HttpCallbackTask(std::function<void(http::Response *)> & cb)
			: mCallback(cb), IRpcTask<http::Response>(0) { }
	private:
		inline void OnResponse(http::Response *response) final;
	private:
		std::function<void(http::Response*)> mCallback;
	};

	inline void HttpCallbackTask::OnResponse(http::Response* response)
	{
		this->mCallback(response);
		delete response;
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
        void OnResponse(http::Response  * response) final;
    private:
        int mRef;
        lua_State * mLua;
    };
}

#endif //APP_HTTPTASK_H
