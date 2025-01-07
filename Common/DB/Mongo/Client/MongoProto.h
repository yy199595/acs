//
// Created by mac on 2022/5/18.
//

#ifndef APP_MONGOPROTO_H
#define APP_MONGOPROTO_H

#include"Proto/Message/IProto.h"
#include"Yyjson/Document/Document.h"
#include"Proto/Document/BsonDocument.h"

#define OP_REPLY 1
#define OP_MSG    1000
#define OP_UPDATE 2001
#define OP_INSERT 2002
#define OP_QUERY 2004
#define OP_GET_MORE    2005
#define OP_DELETE 2006
#define OP_KILL_CURSORS    2007
#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace mongo
{
    class Head
    {
    public:
        int messageLength;
        int requestID;
        int responseTo;
        int opCode;
    };

	class Request final : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<Request>
#endif
    {
    public:
		Request();
	public:
		void Clear() final;
		std::string ToString() final;
		int OnSendMessage(std::ostream &os) final;
		int GetRpcId() const { return this->header.requestID; }
	public:
		Request & GetCollection(const char * cmd, const std::string & tab);
	public:
		Request & Skip(int skip = 1);
		Request & Limit(int limit = 1);
		Request & Query(bson::Writer::Document & doc);
		Request & Filter(bson::Writer::Document & doc);
	public:
		void Insert(bson::Writer::Array & documents);
		void Insert(bson::Writer::Document & document);
	public:
        int flag;
		Head header;
		std::string tab;
		std::string cmd;
		int numberToSkip;
		int numberToReturn;
		std::string dataBase;
		std::string collectionName;
		bson::Writer::Document document;
	};

	class Response final : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<Response>
#endif
    {
    public:
		Response();
		Response(int id) { this->mHead.responseTo = id; }
    public:
		int GetCode() const { return this->mCode; }
		void SetCode(int code) { this->mCode = code; }
		const Head & GetHead() const { return this->mHead;}
		inline int RpcId() const { return this->mHead.responseTo; }
		inline void SetRpcId(int id) { this->mHead.responseTo = id; }
		bson::Reader::Document * Document() const { return this->mDocument.get(); }
	public:
		void Clear() final;
		std::string ToString() final;
#ifdef __DEBUG__
		const std::string& ToJson() const { return this->mJson; }
#endif
		int OnSendMessage(std::ostream &os) final { return 0; }
		int OnRecvMessage(std::istream &os, size_t size) final;
	private:
		int mCode;
        Head mHead;
		int mDecodeState;
        int responseFlags;  // bit vector - see details below
        long long cursorID;       // cursor id if client needs to do get more's
        int startingFrom;   // where in the cursor this reply is starting
        int numberReturned; // number of documents in the reply
		std::string mBuffer;
#ifdef __DEBUG__
		std::string mJson;
#endif
		std::unique_ptr<bson::Reader::Document> mDocument;
		//std::vector<Bson::Reader::Document *> mElements;
    };
}


#endif //APP_MONGOPROTO_H
