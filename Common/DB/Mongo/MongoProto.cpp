//
// Created by mac on 2022/5/18.
//
#include"MongoProto.h"
#include"Bson/bsonobj.h"
namespace Mongo
{
	MongoRequest::MongoRequest(int opcode)
	{
		this->header.requestID = 0;
		this->header.responseTo = 0;
		this->header.opCode = opcode;
		this->header.messageLength = 0;
	}

	int MongoRequest::Serailize(std::ostream& os)
	{
		this->header.messageLength = this->GetLength() + sizeof(MongoHead);
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
	int MongoLuaRequest::GetLength()
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

	int MongoUpdateRequest::GetLength()
	{
		return 0;
	}

	void MongoUpdateRequest::OnWriter(std::ostream& os)
	{

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

	}

	int MongoInsertRequest::GetLength()
	{
		return 0;
	}
}

namespace Mongo
{
	MongoQueryRequest::MongoQueryRequest()
		: MongoRequest(OP_QUERY)
	{

	}

	int MongoQueryRequest::GetLength()
	{
		return sizeof(this->flag) + this->collectionName.size() + 1
			   + sizeof(int) * 2 + this->document.GetStreamLength();
	}
	void MongoQueryRequest::OnWriter(std::ostream& os)
	{
		this->Write(os, this->flag);
		this->Write(os, this->collectionName);
		this->Write(os, this->numberToSkip);
		this->Write(os, this->numberToReturn);
		this->document.WriterToStream(os);
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

	std::shared_ptr<_bson::bsonobj> MongoQueryResponse::GetObject()
	{
		return std::move(this->mBson);
	}

    int MongoQueryResponse::OnReceiveBody(asio::streambuf &buffer)
	{
		this->mBuffer.clear();
		std::iostream os(&buffer);
		os.readsome((char*)&responseFlags, sizeof(responseFlags));
		os.readsome((char*)&cursorID, sizeof(cursorID));
		os.readsome((char*)&startingFrom, sizeof(startingFrom));
		os.readsome((char*)&numberReturned, sizeof(numberReturned));

		os >> this->mBuffer;
		const char * msgData = this->mBuffer.c_str();
		this->mBson = std::make_shared<_bson::bsonobj>(msgData);
		return 0;
	}
}