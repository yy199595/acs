//
// Created by yjz on 2022/6/25.
//

#ifndef _MONGOBSON_H_
#define _MONGOBSON_H_

#define BSON_REAL 1
#define BSON_STRING 2
#define BSON_DOCUMENT 3
#define BSON_ARRAY 4
#define BSON_BINARY 5
#define BSON_UNDEFINED 6
#define BSON_OBJECTID 7
#define BSON_BOOLEAN 8
#define BSON_DATE 9
#define BSON_NULL 10
#define BSON_REGEX 11
#define BSON_DBPOINTER 12
#define BSON_JSCODE 13
#define BSON_SYMBOL 14
#define BSON_CODEWS 15
#define BSON_INT32 16
#define BSON_TIMESTAMP 17
#define BSON_INT64 18
#define BSON_MINKEY 255
#define BSON_MAXKEY 127
#include<string>
#include<vector>
#include<ostream>
#include<istream>
#include"unordered_map"
#include"Json/JsonWriter.h"
namespace Bson
{
	class BsonNode
	{
	 public:
		virtual int GetSize() const = 0;
		virtual char GetType() const = 0;
		virtual void Serialize(std::ostream & oss) = 0;
		template<typename T> T * Cast() { return dynamic_cast<T*>(this); }
	};

	template<typename T, int type>
	class BsonCommandNode : public BsonNode
	{
	 public:
		BsonCommandNode(T val) :mValue(val) {}
	 public:
		char GetType() const final { return type;}
		int GetSize() const final { return sizeof(T) + 1; }
		const T & GetData() const { return this->mValue; }
		void Serialize(std::ostream &oss) final
		{
			char t = type;
			oss.write((char *)&t, sizeof(char));
			oss.write((char *)&mValue, sizeof(mValue));
		}
	 private:
		const T mValue;
	};

	template<int type>
	class BsonStringNode : public BsonNode
	{
	 public:
		BsonStringNode(const std::string & val) : mValue(val) {}
		BsonStringNode(const char * str, size_t size) : mValue(str, size) {}
	 public:
		char GetType() const final { return type;}
		int GetSize() const final { return mValue.size(); }
		const std::string & GetData() const { return this->mValue; }
		void Serialize(std::ostream &oss) final
		{
			int length = this->mValue.size() + 1;
			oss.write((char *)&length, sizeof(int));
			oss << this->mValue << '\0';
		}
	 private:
		const std::string mValue;
	};

	typedef BsonStringNode<BSON_STRING> BsonStrNode;
	typedef BsonStringNode<BSON_BINARY> BsonBinStrNode;
	typedef BsonCommandNode<int, BSON_INT32> BsonInt32Node;
	typedef BsonCommandNode<bool, BSON_BOOLEAN> BsonBoolNode;
	typedef BsonCommandNode<double, BSON_REAL> BsonDoubleNode;
	typedef BsonCommandNode<long long, BSON_INT64> BsonInt64Node;

}


namespace Bson
{
	class IBson
	{
	 protected:
		virtual void AddNode(const std::string & key, BsonNode * node) = 0;
	};

	class BsonEncoder : public IBson
	{
	 private:
		char ReadChar(std::iostream & oss);
		template<typename T>
		bool ReadCommand(std::iostream & oss, T & value)
		{
			if(oss.readsome((char *)&value, sizeof(T)) == sizeof(T))
			{
				this->mMaxSize -= sizeof(T);
				return true;
			}
			return false;
		}
		bool ReadString(std::iostream &oss, std::string & value);
		bool ReadString(std::iostream &oss, std::string & value, size_t size);
	 protected:
		bool Decode(std::iostream & oss, int size);
	 protected:
		int mMaxSize;
	};
}

namespace Bson
{
	class BsonDecoder : public IBson
	{
	 public:
		void Add(const std::string & key, int value);
		void Add(const std::string & key, bool value);
		void Add(const std::string & key, double value);
		void Add(const std::string & key, long long value);
		void Add(const std::string & key, BsonNode  * value);
		void Add(const std::string & key, const char * value);
		void Add(const std::string & key, const std::string & value);
	};
}

namespace Bson
{
	class BsonArrayNode : public BsonNode, public BsonEncoder, public BsonDecoder
	{
	 public:
		BsonArrayNode() : mSize(0) { };
		BsonArrayNode(std::iostream & oss, int maxSize);
	 public:
		void Serialize(std::ostream &oss) final;
		char GetType() const { return BSON_ARRAY;}
		int GetSize() const final { return this->mSize;}
		const BsonNode * GetNode(size_t index) const;
		size_t GetArraySize() const { return this->mNodes.size(); }
	 public:
		std::vector<BsonNode *>::iterator End() { return this->mNodes.end();}
		std::vector<BsonNode *>::iterator Begin() { return this->mNodes.begin();}
	 private:
		void AddNode(const std::string &key, BsonNode *node) final;
	 private:
		size_t mSize;
		std::vector<BsonNode *> mNodes;
	};
}

namespace Bson
{
	typedef std::unordered_map<std::string, BsonNode *>::iterator DocumentIter;
	class BsonDocumentNode : public BsonNode, public BsonEncoder, public BsonDecoder
	{
	 public:
		BsonDocumentNode() : mSize(4) {};
		BsonDocumentNode(std::iostream & oss, int size);
	 public:
		void Serialize(std::ostream &oss) final;
		int GetSize() const final { return mSize;}
		char GetType() const { return BSON_DOCUMENT;}
		const BsonNode * GetNode(const std::string & key) const;
	 public:
		void WriterToJson(std::string & json);
		DocumentIter End() { return this->mNodes.end(); }
		DocumentIter Begin() { return this->mNodes.begin(); }
	 private:
		void WriterToJson(BsonNode * bsonNode, Json::Writer & json);
		void WriterToJson(BsonArrayNode * bsonNode, Json::Writer & json);
		void WriterToJson(BsonDocumentNode * bsonNode, Json::Writer & json);
		void AddNode(const std::string &key, BsonNode *node) final;
	 private:
		size_t mSize;
		std::unordered_map<std::string, BsonNode *> mNodes;
	};
}

#endif //_MONGOBSON_H_
