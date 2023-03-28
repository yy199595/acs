//
// Created by yjz on 2022/10/27.
//

#include"HttpResponse.h"
#include"Json/JsonWriter.h"
namespace Http
{
    Response::Response()
    {
        this->mVersion = HttpVersion;
        this->mParseState = DecodeState::None;
    }

    int Response::Serialize(std::ostream &buffer)
    {
        return this->OnWrite(buffer);
    }

    bool Response::OnRead(std::istream &buffer)
    {
        if(this->mParseState == DecodeState::None)
        {
            buffer >> this->mVersion;
            buffer >> this->mCode;
            buffer >> this->mError;
            buffer.ignore(2); //放弃\r\n
            this->mParseState = DecodeState::Head;
        }
        if(this->mParseState == DecodeState::Head)
        {
            if(!this->mHead.OnRead(buffer))
            {
                return false;
            }
            this->mParseState = DecodeState::Body;
        }
        if(this->mParseState == DecodeState::Body)
        {
            char buff[128] = {0};
            size_t size = buffer.readsome(buff, sizeof(buff));
            while(size > 0)
            {
                this->mContent.append(buff, size);
                size = buffer.readsome(buff, sizeof(buff));
            }
        }
        return false;
    }

    int Response::OnWrite(std::ostream &buffer)
    {
        buffer << this->mVersion << ' ' << (int)this->mCode << ' '
                << HttpStatusToString((HttpStatus)this->mCode) << "\r\n";
        this->mHead.OnWrite(buffer);

        buffer << "\r\n";
        buffer.write(this->mContent.c_str(), this->mContent.size());
        return 0;
    }

    void Response::Str(HttpStatus code, const std::string &str)
    {
        this->mCode = (int)code;
        this->mContent.assign(str.c_str(), str.size());
		this->mHead.Add("content-type", HttpContentType::STRING);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

    void Response::Json(HttpStatus code, Json::Writer& doc)
    {
        this->mCode = (int)code;
        doc.WriterStream(&mContent);
        this->mHead.Add("content-type", HttpContentType::JSON);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

	void Response::Html(HttpStatus code, const std::string& html)
	{
		this->mCode = (int)code;
		this->mContent.assign(html);
		this->mHead.Add("content-type", HttpContentType::HTML);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

    void Response::Json(HttpStatus code, const std::string &json)
    {
       this->Json(code, json.c_str(), json.size());
    }

    void Response::Json(HttpStatus code, const char *str, size_t len)
    {
        this->mCode = (int)code;
        this->mContent.assign(str, len);
        this->mHead.Add("content-type", HttpContentType::JSON);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

    void Response::Content(HttpStatus code, const std::string& type, const std::string& str)
    {
        this->mCode = (int)code;
        this->mContent.assign(str);
        this->mHead.Add("content-type", type);
        this->mHead.Add("content-length", (int)this->mContent.size());
    }
}