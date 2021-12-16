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
        explicit HttpPostRequest(const std::string & url, const std::string & content);
        ~HttpPostRequest() override = default;
    protected:
        void WriteToSendBuffer(std::ostream &os) final;
    private:
        std::string mResponse;
        std::string mWriteContent;
    };
}
#endif //GameKeeper_HTTPLOCALPOSTREQUEST_H
