//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOPROTO_H
#define SERVER_MONGOPROTO_H
#include"asio/streambuf.hpp"
#include"Json/JsonWriter.h"
#include"DB/Mongo/BsonDocument.h"
#include"Network/Proto/ProtoMessage.h"

#define OP_REPLY 1
#define OP_MSG    1000
#define OP_UPDATE 2001
#define OP_INSERT 2002
#define OP_QUERY 2004
#define OP_GET_MORE    2005
#define OP_DELETE 2006
#define OP_KILL_CURSORS    2007

namespace Mongo
{
    class MongoHead
    {
    public:
        int messageLength;
        int requestID;
        int responseTo;
        int opCode;
    };

    class MongoRequest : public Tcp::ProtoMessage
    {
    public:
        MongoRequest(int opcode);

    public:
        int Serailize(std::ostream &os) final;
    protected:
        virtual int GetLength() = 0;

        virtual void OnWriter(std::ostream &os) = 0;

    public:
        MongoHead header;
    };

    class MongoUpdateRequest : public MongoRequest
    {
    public:
        MongoUpdateRequest();

    protected:
        int GetLength() final;

        void OnWriter(std::ostream &os) final;

    public:
        int zero;
        std::string collectionName;
        int flag;
        Bson::WriterDocument selector;
		Bson::WriterDocument update;
    };

    class MongoInsertRequest : public MongoRequest
    {
    public:
        MongoInsertRequest();

    protected:
        int GetLength() final;

        void OnWriter(std::ostream &os) final;

    public:
        int zero;
        std::string collectionName;
		Bson::WriterDocument document;
    };

	class MongoLuaRequest : public MongoRequest
	{
	public:
		MongoLuaRequest() : MongoRequest(OP_QUERY) {}
	private:
		int GetLength();
		void OnWriter(std::ostream &os);
	public:
		std::string mCommand;
	};

    class MongoQueryRequest : public MongoRequest
    {
    public:
        MongoQueryRequest();

    protected:
        int GetLength() final;

        void OnWriter(std::ostream &os) final;

    public:
        int flag;
        std::string collectionName;
        int numberToSkip;
        int numberToReturn;
		Bson::WriterDocument document;
    };

    class MongoQueryResponse
    {
    public:
        MongoQueryResponse() = default;

    public:
        int OnReceiveHead(asio::streambuf &buffer);
        int OnReceiveBody(asio::streambuf &buffer);
		std::shared_ptr<_bson::bsonobj> GetObject();
		const MongoHead & GetHead() const { return this->mHead;}
	private:
        MongoHead mHead;
        int responseFlags;  // bit vector - see details below
        long long cursorID;       // cursor id if client needs to do get more's
        int startingFrom;   // where in the cursor this reply is starting
        int numberReturned; // number of documents in the reply
		std::string mBuffer;
        std::shared_ptr<_bson::bsonobj> mBson;
    };
}


#endif //SERVER_MONGOPROTO_H
