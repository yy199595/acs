//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPLOCALPOSTREQUEST_H
#define GameKeeper_HTTPLOCALPOSTREQUEST_H
#include "HttpRequest.h"
namespace GameKeeper
{
	class HttpWriteContent;
    class HttpPostRequest : public HttpRequest
    {
    public:
        explicit HttpPostRequest(HttpClientComponent * component);
        ~HttpPostRequest() override = default;
    public:
        HttpMethodType GetType() final { return HttpMethodType::POST; }
        XCode Post(const std::string & url, HttpWriteContent & content , std::string & response);
    protected:
        bool WriterToBuffer(std::ostream & os) override;
        void OnReceiveBody(asio::streambuf & buf) override;
    private:
        unsigned int mCorId;
        std::string * mResponse;
        HttpWriteContent * mPostContent;
    };
}
#endif //GameKeeper_HTTPLOCALPOSTREQUEST_H
