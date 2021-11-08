//
// Created by zmhy0073 on 2021/11/4.
//

#include "HttpPostHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpPostHandler::HttpPostHandler(HttpClientComponent *component)
        : HttpRequestHandler(component)
    {

    }

    const std::string &HttpPostHandler::GetPath()
    {
		return this->mPath;
    }

    void HttpPostHandler::Clear()
    {
        HttpRequestHandler::Clear();
        this->mPath.clear();
        this->mVersion.clear();
        if(this->mStreamBuf.size() > 0)
        {
            std::istream is(&this->mStreamBuf);
            is.ignore(this->mStreamBuf.size());
        }
    }

	bool HttpPostHandler::OnReceiveHeard(asio::streambuf &streamBuf)
	{
		std::istream is(&streamBuf);
		is >> this->mPath >> this->mVersion;
        this->ParseHeard(streamBuf);

        if(streamBuf.size() > 0)
        {
            std::ostream os(&this->mStreamBuf);
            os.write(asio::buffer_cast<const char *>(streamBuf.data()), streamBuf.size());

            is.ignore(streamBuf.size());
        }

		auto protocolComponent = App::Get().GetComponent<ProtocolComponent>();
		this->mHttpConfig = protocolComponent->GetHttpConfig(this->GetPath());
		return true;
	}

    size_t HttpPostHandler::ReadFromStream(char *buffer, size_t size)
    {
        if(this->mStreamBuf.size() > 0)
        {
            std::istream is(&this->mStreamBuf);
            return is.readsome(buffer, size);
        }
        return 0;
    }
}

