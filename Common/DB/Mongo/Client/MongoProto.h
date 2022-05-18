//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOPROTO_H
#define SERVER_MONGOPROTO_H
#include"asio/streambuf.hpp"
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
		bool Serailize(std::ostream& os) final;
		void WriteBson(std::ostream& os, minibson::document& document);
	 protected:
		virtual int GetLength() = 0;
		virtual void OnWriter(std::ostream& os) = 0;
	 public:
		MongoHead header;
	};

	class MongoUpdateRequest : MongoRequest
	{
	 public:
		MongoUpdateRequest();
	 protected:
		int GetLength() final;
		void OnWriter(std::ostream& os) final;
	 public:
		int zero;
		std::string collectionName;
		int flag;
		minibson::document selector;
		minibson::document update;
	};
}


#endif //SERVER_MONGOPROTO_H
