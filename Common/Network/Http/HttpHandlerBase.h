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
        virtual void Clear();
        virtual HttpMethodType GetType() = 0;
		virtual bool WriterToBuffer(std::ostream & os) = 0;
		virtual bool OnReceiveHead(asio::streambuf & buf) = 0;

#ifdef __DEBUG__
		std::string PrintHeard();
#endif // __DEBUG__	
	protected:
		void ParseHeard(asio::streambuf & buf);
	public:
		size_t GetContentLength() const { return this->mContentLength; }
		bool GetHeardData(const std::string & key, std::string & value);
	private:
        size_t mContentLength;
		std::unordered_map<std::string, std::string> mHeardMap;
    protected:
        char mHandlerBuffer[1024];
	};
}