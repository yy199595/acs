//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOPROTO_H
#define SERVER_MONGOPROTO_H
#include"asio/streambuf.hpp"

#include"BsonDocument.h"
#include"Util/Json/JsonWriter.h"
#include"Proto/Message/ProtoMessage.h"

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
        int Serialize(std::ostream &os) final;
    protected:
        virtual int GetLength() = 0;

        virtual void OnWriter(std::ostream &os) = 0;

    public:
        MongoHead header;
    };

    class CommandRequest : public MongoRequest
    {
    public:
        CommandRequest();

    protected:
        int GetLength() final;

        void OnWriter(std::ostream &os) final;

    public:
        int flag;
        int mTaskId;
        int numberToSkip;
        int numberToReturn;
        std::string dataBase;
        std::string collectionName;
		Bson::Writer::Document document;
	};

    class CommandResponse : public std::vector<Bson::Reader::Document *>
    {
    public:
        CommandResponse() = default;
        ~CommandResponse();
    public:
        int OnReceiveHead(std::istream & stream);
		size_t OnReceiveBody(std::istream & stream);
        const MongoHead & GetHead() const { return this->mHead;}
        size_t GetDocumentSize() const { return this->size();}
        Bson::Reader::Document & Get(size_t index = 0) const { return *(this->at(index));}
        Bson::Reader::Document & operator[](size_t index) const { return *(this->at(index));}
    private:
        int ReadInt(std::istream & is);
        long long ReadLong(std::istream & is);

    private:
        MongoHead mHead;
        int responseFlags;  // bit vector - see details below
        long long cursorID;       // cursor id if client needs to do get more's
        int startingFrom;   // where in the cursor this reply is starting
        int numberReturned; // number of documents in the reply
		std::string mBuffer;
		char mReadBuffer[128];
    };
}


#endif //SERVER_MONGOPROTO_H
