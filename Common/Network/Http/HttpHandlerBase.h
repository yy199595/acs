#pragma once
#include <Define/CommonTypeDef.h>
namespace Sentry
{
	class HttpHandlerBase
	{
	public:
		HttpHandlerBase() = default;
		virtual ~HttpHandlerBase() = default;
	public:
		virtual bool WriterToBuffer(std::ostream & os) { }
		virtual bool OnSessionError(const asio::error_code & code) = 0;
		virtual bool OnReceiveBody(asio::streambuf & buf, const asio::error_code & code) = 0;
		virtual bool OnReceiveHeard(asio::streambuf & buf, const asio::error_code & code) = 0;
	protected:
		void ParseHeard(asio::streambuf & buf);
		bool GetHeardData(const std::string & key, std::string & value);
	private:
		std::unordered_map<std::string, std::string> mHeardMap;
	};
}