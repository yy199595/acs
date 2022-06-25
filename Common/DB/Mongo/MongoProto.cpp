//
// Created by mac on 2022/5/18.
//

#include "MongoProto.h"
#include"DB/Mongo/Bson/MongoBson.h"
namespace Mongo
{
	MongoRequest::MongoRequest(int opcode)
	{
		this->header.requestID = 0;
		this->header.responseTo = 0;
		this->header.opCode = opcode;
		this->header.messageLength = 0;
	}


	void MongoRequest::WriteBson(std::ostream & os,minibson::document& document)
	{
		size_t size = document.get_serialized_size();
		std::unique_ptr<char []> buffer(new char[size]);
		document.serialize(buffer.get(), size);
		std::string bson(buffer.get(), size);
		os.write(buffer.get(), size);
	}

	int MongoRequest::Serailize(std::ostream& os)
	{
		this->header.messageLength = this->GetMessageSize();
		this->Write(os, this->header.messageLength);
		this->Write(os, this->header.requestID);
		this->Write(os, this->header.responseTo);
		this->Write(os, this->header.opCode);
		this->OnWriter(os);
		return 0;
	}
}

namespace Mongo
{
	int MongoLuaRequest::GetMessageSize()
	{
		return this->mCommand.size();
	}

	void MongoLuaRequest::OnWriter(std::ostream& os)
	{
		os.write(this->mCommand.c_str(), this->mCommand.size());
	}

}

namespace Mongo
{
	MongoUpdateRequest::MongoUpdateRequest()
		: MongoRequest(OP_UPDATE)
	{
		this->flag = 0;
		this->zero = 0;
	}

	int MongoUpdateRequest::GetMessageSize() const
	{
		return sizeof(this->zero) + this->collectionName.size()
		 + sizeof(this->flag) + this->selector.get_serialized_size()
		  + this->update.get_serialized_size();
	}

	void MongoUpdateRequest::OnWriter(std::ostream& os)
	{
		this->Write(os, this->zero);
		this->Write(os, this->collectionName);
		this->Write(os, this->flag);
		this->WriteBson(os, this->selector);
		this->WriteBson(os, this->update);
	}
}

namespace Mongo
{
	MongoInsertRequest::MongoInsertRequest()
		: MongoRequest(OP_INSERT)
	{

	}

	void MongoInsertRequest::OnWriter(std::ostream& os)
	{
		this->Write(os, this->zero);
		this->Write(os, this->collectionName);
		this->WriteBson(os, this->document);
	}

	int MongoInsertRequest::GetMessageSize() const
	{
		return sizeof(this->zero) +
			this->collectionName.size() + this->document.get_serialized_size();
	}
}

namespace Mongo
{
	MongoQueryRequest::MongoQueryRequest()
		: MongoRequest(OP_QUERY)
	{

	}

	int MongoQueryRequest::GetMessageSize() const
	{
		size_t size = sizeof(MongoHead);
		size += (this->collectionName.size() + 1);
		size += (sizeof(this->flag) + sizeof(int) * 2);
		size += this->document.GetSize();
		return size;
	}

	void MongoQueryRequest::OnWriter(std::ostream& os)
	{
		this->Write(os, this->flag);
		this->Write(os, this->collectionName);
		this->Write(os, this->numberToSkip);
		this->Write(os, this->numberToReturn);
		this->document.Serialize(os);
	}
}

namespace Mongo
{
    int MongoQueryResponse::OnReceiveHead(asio::streambuf &buffer)
    {
        std::iostream os(&buffer);
        os.readsome((char *)&this->mHead, sizeof(MongoHead));
        return this->mHead.messageLength;
    }

    int MongoQueryResponse::OnReceiveBody(asio::streambuf &buffer)
    {
        std::iostream os(&buffer);
        os.readsome((char*)&responseFlags, sizeof(responseFlags));
        os.readsome((char*)&cursorID, sizeof(cursorID));
        os.readsome((char*)&startingFrom, sizeof(startingFrom));
        os.readsome((char*)&numberReturned, sizeof(numberReturned));


        int length = 0;
		os.readsome((char *)&length, sizeof(int));

		std::string json;
		Bson::BsonDocumentNode document(os, length);
		document.WriterToJson(json);

		document.GetNode("ismaster");
        return 0;
    }
}