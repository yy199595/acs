//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPLOCALPOSTREQUEST_H
#define GameKeeper_HTTPLOCALPOSTREQUEST_H
#include "HttpRequest.h"
namespace GameKeeper
{
    class HttpReadContent;
	class HttpWriteContent;
    class HttpPostRequest : public HttpRequest
    {
    public:
        explicit HttpPostRequest(HttpComponent * component);
        ~HttpPostRequest() override = default;
    public:
        void Clear() override;
        HttpMethodType GetType() final { return HttpMethodType::POST; }
        bool Init(const std::string & url, HttpWriteContent & request, HttpReadContent & response);
    protected:
        void WriteHead(std::ostream &os) final;
        bool WriteBody(std::ostream &os) final;
        void OnReceiveBody(asio::streambuf & buf) final;
    private:
        HttpReadContent * mReadContent;
        HttpWriteContent * mWriteContent;
    };
}
#endif //GameKeeper_HTTPLOCALPOSTREQUEST_H
