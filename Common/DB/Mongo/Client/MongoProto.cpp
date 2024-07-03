//
// Created by mac on 2022/5/18.
//
#include"MongoProto.h"
#include"Proto/Bson/bsonobj.h"

namespace mongo
{
	Request::Request()
	{
		this->flag = 0;
		this->numberToSkip  = 0;
		this->numberToReturn = 1;
		this->header.requestID = 0;
		this->header.responseTo = 0;
		this->header.opCode = OP_QUERY;
		this->header.messageLength = 0;
	}

	void Request::Clear()
	{
		this->flag = 0;
		this->numberToSkip  = 0;
		this->numberToReturn = 1;
		this->header.requestID = 0;
		this->header.responseTo = 0;
		this->header.opCode = OP_QUERY;
		this->header.messageLength = 0;
	}

	std::string Request::ToString()
	{
		return this->document.ToString();
	}

	int Request::OnSendMessage(std::ostream& os)
	{
		int len = sizeof(this->flag) + this->collectionName.size()
			   + 1 + sizeof(int) * 2 + this->document.GetStreamLength();

		len  += sizeof(Head);
		tcp::Data::Write(os, len);
		tcp::Data::Write(os, this->header.requestID);
		tcp::Data::Write(os, this->header.responseTo);
		tcp::Data::Write(os, this->header.opCode);

		tcp::Data::Write(os, this->flag);
		tcp::Data::Write(os, this->collectionName);
		tcp::Data::Write(os, '\0');
		tcp::Data::Write(os, this->numberToSkip);
		tcp::Data::Write(os, this->numberToReturn);
		this->document.Encode(os);

		return 0;
	}

	Request& Request::GetCollection(const char* cmd, const std::string& tab)
	{
		this->tab = tab;
		this->cmd = cmd;
		this->document.Add(cmd, tab);
		return *this;
	}
}

namespace mongo
{
	void Request::Insert(bson::Writer::Array& documents)
	{
//		bson::Writer::Array documents;
//		documents.Add(doc);
		this->document.Add("documents", documents);
	}

	void Request::Insert(bson::Writer::Document& document)
	{
		bson::Writer::Array documents;
		documents.Add(document);
		this->document.Add("documents", documents);
	}
}

namespace mongo
{

	Request& Request::Skip(int skip)
	{
		this->document.Add("skip", skip);
		return *this;
	}

	Request& Request::Limit(int limit)
	{
		this->document.Add("limit", limit);
		return *this;
	}

	Request& Request::Query(bson::Writer::Document& doc)
	{
		this->document.Add("query", doc);
		return *this;
	}

	Request& Request::Filter(bson::Writer::Document& filter)
	{
//		bson::Writer::Document mode;
//		mode.Add("mode", "secondaryPreferred");
//		this->document.Add("$readPreference", mode);
		this->document.Add("filter", filter);
		return *this;
	}
}


namespace mongo
{
	Response::Response()
	{
		this->mCode = 0;
		this->mDecodeState = tcp::Decode::None;
	}

	void Response::Clear()
	{
		this->mCode = 0;
		this->mBuffer.clear();
		this->mDecodeState = tcp::Decode::None;
	}

	std::string Response::ToString()
	{
		if(this->mDocument == nullptr)
		{
			return "";
		}
		return this->mDocument->ToString();
	}

	int Response::OnRecvMessage(std::istream& os, size_t size)
	{
		if(this->mDecodeState == tcp::Decode::None)
		{
			tcp::Data::Read(os, this->mHead);
			this->mDecodeState = tcp::Decode::MessageHead;
			return this->mHead.messageLength - (int)sizeof(this->mHead);
		}
		if(this->mDecodeState == tcp::Decode::MessageHead)
		{
			this->mBuffer.clear();
			tcp::Data::Read(os, this->responseFlags);
			tcp::Data::Read(os, this->cursorID);
			tcp::Data::Read(os, this->startingFrom);
			tcp::Data::Read(os, this->numberReturned);
			this->mDecodeState = tcp::Decode::MessageBody;
		}
		if(this->mDecodeState == tcp::Decode::MessageBody)
		{
			static thread_local char buffer[256] = { 0 };
			size_t size = os.readsome(buffer, sizeof(buffer));
			while(size > 0)
			{
				this->mBuffer.append(buffer, size);
				size = os.readsome(buffer, sizeof(buffer));
			}
			const char * bson = this->mBuffer.c_str();
			this->mDocument = std::make_unique<bson::Reader::Document>(bson);
#ifdef __DEBUG__
			this->mDocument->WriterToJson(&this->mJson);
#endif
		}
		return 0;
	}
}