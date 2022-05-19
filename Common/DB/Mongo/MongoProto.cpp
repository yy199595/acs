//
// Created by mac on 2022/5/18.
//

#include "MongoProto.h"

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
		os.write(buffer.get(), size);
	}

	bool MongoRequest::Serailize(std::ostream& os)
	{
		this->header.messageLength = sizeof(this->header) - 4 + this->GetLength();
		this->Write(os, this->header.messageLength);
		this->Write(os, this->header.requestID);
		this->Write(os, this->header.responseTo);
		this->Write(os, this->header.opCode);
		this->OnWriter(os);
		return true;
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

	int MongoInsertRequest::GetLength()
	{
		return sizeof(this->zero) + this->collectionName.size()
			 + this->document.get_serialized_size();
	}
}