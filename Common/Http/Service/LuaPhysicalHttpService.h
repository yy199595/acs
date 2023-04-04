//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LOCALLUAHTTPSERVICE_H
#define SERVER_LOCALLUAHTTPSERVICE_H
#include"HttpService.h"
namespace Tendo
{
    class LuaPhysicalHttpService : public HttpService
    {
    public:
        LuaPhysicalHttpService();
        ~LuaPhysicalHttpService() = default;
    private:
		bool OnInit() final;
		bool OnStart() final { return true; }
		bool OnClose() final { return true; }
		bool IsStartService() final { return true;}
		int Invoke(const std::string & name,
			const std::shared_ptr<Http::Request> &, std::shared_ptr<Http::DataResponse> &) final;
	private:
		unsigned int mSumCount;
		unsigned int mWaitCount;
    };
}


#endif //SERVER_LOCALLUAHTTPSERVICE_H
