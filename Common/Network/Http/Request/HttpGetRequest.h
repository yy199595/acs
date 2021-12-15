#pragma once
#include "HttpRequest.h"
namespace GameKeeper
{
	class HttpReadContent;
	class HttpComponent;
	class HttpGetRequest : public HttpRequest
	{
	public:
		explicit HttpGetRequest(HttpComponent * component);
		~HttpGetRequest() override = default;
	public:
        void Clear() override;
        bool Init(const std::string & url, HttpReadContent * response);
        HttpMethodType GetType() final { return HttpMethodType::GET; }
	protected:
        void WriteHead(std::ostream &os) final;
        bool WriteBody(std::ostream &os) final;
        void OnReceiveBody(asio::streambuf & buf) override;
	private:
        HttpReadContent * mResponse;
#ifdef __DEBUG__
		size_t mCurrentLength;
#endif // __DEBUG__

	};
}