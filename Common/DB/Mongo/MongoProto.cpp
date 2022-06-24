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
		std::string bson(buffer.get(), size);
		os.write(buffer.get(), size);
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

	int MongoQueryRequest::GetLength()
	{
		return sizeof(this->flag) + this->collectionName.size()
			   + sizeof(int) * 2 + this->command.size() + 4;
	}
	void MongoQueryRequest::OnWriter(std::ostream& os)
	{
		this->Write(os, this->flag);
		this->Write(os, this->collectionName);
		this->Write(os, this->numberToSkip);
		this->Write(os, this->numberToReturn);
		this->Write(os, (int)this->command.size());
		os.write(this->command.c_str(), this->command.size());
		//this->WriteBson(os, this->document);
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

    void MongoQueryResponse::CastJson(const std::string & name, minibson::node *node)
    {
        minibson::document *document1 = dynamic_cast<minibson::document *>(node);
        if (document1 != nullptr)
        {
            this->mJsonWriter.StartObject(name.c_str());
            for (auto iter = document1->begin(); iter != document1->end(); iter++)
            {
                const std::string &name1 = iter->first;
                this->CastJson(name1, iter->second);
            }
            this->mJsonWriter.EndObject();
            return;
        }
        minibson::int32 *int321 = dynamic_cast<minibson::int32 *>(node);
        minibson::int64 *int641 = dynamic_cast<minibson::int64 *>(node);
        minibson::string *string1 = dynamic_cast<minibson::string *>(node);
        minibson::Double *aDouble = dynamic_cast<minibson::Double *>(node);
        if (int321 != nullptr)
        {
            this->mJsonWriter.AddMember(name.c_str(), int321->get_value());
        }
        else if (int641 != nullptr)
        {
            this->mJsonWriter.AddMember(name.c_str(), int641->get_value());

        }
        else if (string1 != nullptr)
        {
            this->mJsonWriter.AddMember(name.c_str(), string1->get_value());

        }
        else if (aDouble != nullptr)
        {
            this->mJsonWriter.AddMember(name.c_str(), aDouble->get_value());
        }

    }

    int MongoQueryResponse::OnReceiveBody(asio::streambuf &buffer)
    {
        std::iostream os(&buffer);
        os.readsome((char*)&responseFlags, sizeof(responseFlags));
        os.readsome((char*)&cursorID, sizeof(cursorID));
        os.readsome((char*)&startingFrom, sizeof(startingFrom));
        os.readsome((char*)&numberReturned, sizeof(numberReturned));

        char * buf = new char[buffer.size()];
        size_t ss = os.readsome(buf, buffer.size());
        this->mDocument = new minibson::document(buf, ss);

        for(auto iter = this->mDocument->begin(); iter != this->mDocument->end(); iter++)
        {
            this->CastJson(iter->first, iter->second);
        }
        std::string json;
        this->mJsonWriter.WriterStream(json);
        std::cout << json << std::endl;
        return 0;
    }
}