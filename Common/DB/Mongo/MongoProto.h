//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOPROTO_H
#define SERVER_MONGOPROTO_H
#include"asio/streambuf.hpp"
#include"Json/JsonWriter.h"
#include"DB/Mongo/Bson/MongoBson.h"
#include"DB/Mongo/Bson/minibson.hpp"
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

        void WriteBson(std::ostream &os, minibson::document &document);

    protected:
		virtual int GetMessageSize() const = 0;
		virtual int GetLength() final { return 0; }
        virtual void OnWriter(std::ostream &os) = 0;

    public:
        MongoHead header;
    };

    class MongoUpdateRequest : public MongoRequest
    {
    public:
        MongoUpdateRequest();

    protected:
        int GetMessageSize() const final;

        void OnWriter(std::ostream &os) final;

    public:
        int zero;
        std::string collectionName;
        int flag;
        minibson::document selector;
        minibson::document update;
    };

    class MongoInsertRequest : public MongoRequest
    {
    public:
        MongoInsertRequest();

    protected:
        int GetMessageSize() const final;

        void OnWriter(std::ostream &os) final;

    public:
        int zero;
        std::string collectionName;
        minibson::document document;
    };

	class MongoLuaRequest : public MongoRequest
	{
	public:
		MongoLuaRequest() : MongoRequest(OP_QUERY) {}
	private:
		int GetMessageSize();
		void OnWriter(std::ostream &os);
	public:
		std::string mCommand;
	};

    class MongoQueryRequest : public MongoRequest
    {
    public:
        MongoQueryRequest();

    protected:
        int GetMessageSize() const final;

        void OnWriter(std::ostream &os) final;

    public:
        int flag;
        std::string collectionName;
        int numberToSkip;
        int numberToReturn;
		Bson::BsonDocumentNode document;
    };

    class MongoQueryResponse
    {
    public:
        MongoQueryResponse() = default;

    public:
        int OnReceiveHead(asio::streambuf &buffer);
        int OnReceiveBody(asio::streambuf &buffer);
    private:
        MongoHead mHead;
        int responseFlags;  // bit vector - see details below
        long long cursorID;       // cursor id if client needs to do get more's
        int startingFrom;   // where in the cursor this reply is starting
        int numberReturned; // number of documents in the reply
    };
}


#endif //SERVER_MONGOPROTO_H
