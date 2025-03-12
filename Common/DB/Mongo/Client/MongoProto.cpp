//
// Created by mac on 2022/5/18.
//
#include "fmt.h"
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
		int length = 0;
		std::string cmd = fmt::format("{}.$cmd", this->dataBase);
		const char * bson = this->document.Serialize(length);
		int len = sizeof(this->flag) + cmd.size()
			   + 1 + sizeof(int) * 2 + length;

		len  += sizeof(Head);
		tcp::Data::Write(os, len);
		tcp::Data::Write(os, this->header.requestID);
		tcp::Data::Write(os, this->header.responseTo);
		tcp::Data::Write(os, this->header.opCode);

		tcp::Data::Write(os, this->flag);
		tcp::Data::Write(os, cmd);
		tcp::Data::Write(os, '\0');
		tcp::Data::Write(os, this->numberToSkip);
		tcp::Data::Write(os, this->numberToReturn);
		os.write(bson, length);

		return 0;
	}

	Request& Request::GetCollection(const char* cmd, const std::string& tab)
	{
		this->cmd = cmd;
		size_t  pos = tab.find('.');
		if(pos != std::string::npos)
		{
			this->dataBase = tab.substr(0, pos);
			std::string collection = tab.substr(pos + 1);
			this->document.Add(cmd, collection);
			return *this;
		}
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
	Response::Response(const std::string & cmd)
			: cmd(cmd)
	{
		this->mCode = 0;
		this->mDecodeState = tcp::Decode::None;
	}

	Response::Response(int id, const std::string& cmd)
		: cmd(cmd)
	{
		this->mHead.responseTo = id;
	}

	std::string Response::ToString()
	{
		return this->mDocument.ToString();
	}

	bool Response::Encode(std::string* json)
	{
		return this->mDocument.WriterToJson(json);
	}

	bool Response::DecodeQuery()
	{
		std::unique_ptr<bson::Reader::Document> document1;
		if(!this->mDocument.Get("cursor", document1))
		{
			std::string str = this->mDocument.ToString();
			return false;
		}

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		if(!document1->Get("firstBatch", results))
		{
			if(!document1->Get("nextBatch", results))
			{
				return false;
			}
		}
		std::string json;
		document1->Get("id", this->cursorID);
		for(std::unique_ptr<bson::Reader::Document> & document : results)
		{
			json.clear();
			if(document->WriterToJson(&json))
			{
				this->mResult.emplace_back(json);
			}
		}
		return true;
	}

	int Response::OnRecvMessage(std::istream& os, size_t size)
	{
		if (this->mDecodeState == tcp::Decode::None)
		{
			tcp::Data::Read(os, this->mHead);
			this->mDecodeState = tcp::Decode::MessageHead;
			return this->mHead.messageLength - (int)sizeof(this->mHead);
		}

		size_t offset = 0;
		{
			tcp::Data::Read(os, this->responseFlags);
			offset += sizeof(this->responseFlags);

			tcp::Data::Read(os, this->cursorID);
			offset += sizeof(this->cursorID);

			tcp::Data::Read(os, this->startingFrom);
			offset += sizeof(this->startingFrom);

			tcp::Data::Read(os, this->numberReturned);
			offset += sizeof(this->numberReturned);
		}
		size_t len = size - offset;
		this->mBuffer = std::make_unique<char[]>(len);
		{
			os.readsome(this->mBuffer.get(), len);
			this->mDocument.Init(this->mBuffer.get());
			if(this->cmd == "find" || this->cmd == "aggregate" || this->cmd == "getMore")
			{
				this->DecodeQuery();
			}
		}
		return 0;
	}
}