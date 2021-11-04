//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEGETREQUEST_H
#define GameKeeper_HTTPREMOTEGETREQUEST_H
#include "HttpRemoteRequestHandler.h"

namespace GameKeeper
{
    class HttpRemoteGetRequestHandler : public HttpRemoteRequestHandler
    {
    public:
        using HttpRemoteRequestHandler::HttpRemoteRequestHandler;
        ~HttpRemoteGetRequestHandler() override = default;
    public:
        
		bool SplitParameter(std::unordered_map<std::string, std::string> & parames);

		const std::string & GetParamater() { return this->mParamater; }

		void OnReceiveHeardAfter(XCode code) override;

		bool OnReceiveHeard(asio::streambuf & buf, size_t size) override;

    protected:

        void OnReceiveBodyAfter(XCode code) override { assert(false); }

        void OnReceiveBody(asio::streambuf &buf) override { assert(false);}
      
    private: // 请求参数
		std::string mParamater;		
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
