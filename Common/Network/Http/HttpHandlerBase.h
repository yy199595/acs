#pragma once
#include <XCode/XCode.h>
#include <Define/CommonTypeDef.h>
#include <Network/Http/Http.h>


namespace GameKeeper
{
	class HttpHandlerBase
	{
	public:
		HttpHandlerBase() = default;
		virtual ~HttpHandlerBase() = default;
	public:
        virtual HttpMethodType GetType() = 0;
        virtual void OnWriterAfter(XCode code) = 0;
        virtual void OnReceiveBodyAfter(XCode code) = 0;
        virtual void OnReceiveHeardAfter(XCode code) = 0;
		virtual bool WriterToBuffer(std::ostream & os) = 0;
        virtual void OnReceiveBody(asio::streambuf & buf) = 0;
		virtual bool OnReceiveHeard(asio::streambuf & buf, size_t size) = 0;
	protected:
		void ParseHeard(asio::streambuf & buf, size_t size);
	public:
		size_t GetContentLength() const { return this->mContentLength; }
		bool GetHeardData(const std::string & key, std::string & value);
	private:
		std::unordered_map<std::string, std::string> mHeardMap;
    protected:
        char mHandlerBuffer[1024];
	private:
		size_t mContentLength;
	};
}