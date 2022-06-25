//
// Created by yjz on 2022/6/25.
//

#include "MongoBson.h"
#include"Json/JsonWriter.h"

namespace Bson
{
	bool BsonEncoder::ReadString(std::iostream& oss, std::string& value, size_t size)
	{
		std::unique_ptr<char[]> buffer(new char[size]);
		if (oss.readsome(buffer.get(), size) == size)
		{
			this->mMaxSize -= size;
			value.append(buffer.get(), size - 1);
			return true;
		}
		return false;
	}

	bool BsonEncoder::ReadString(std::iostream& oss, std::string& value)
	{
		char cc = oss.get();
		while (cc != '\0')
		{
			value += cc;
			cc = oss.get();
		}
		this->mMaxSize -= 1;
		this->mMaxSize -= value.size();
		return true;
	}

	char BsonEncoder::ReadChar(std::iostream& oss)
	{
		this->mMaxSize--;
		return oss.get();
	}

	bool BsonEncoder::Decode(std::iostream& oss, int size)
	{
		this->mMaxSize = size;
		char bt = this->ReadChar(oss);
		while(this->mMaxSize > 0 && bt != '\0')
		{
			std::string key;
			this->ReadString(oss, key);
			printf("key = %s\n", key.c_str());
			BsonNode * bsonNode = nullptr;
			switch((int)bt)
			{
			case BSON_REAL:
			{
				double value = 0;
				this->ReadCommand(oss, value);
				bsonNode = new BsonDoubleNode(value);
			}
				break;
			case BSON_INT32:
			{
				int value = 0;
				this->ReadCommand(oss, value);
				bsonNode = new BsonInt32Node(value);
			}
				break;

			case BSON_INT64:
			{
				long long value = 0;
				this->ReadCommand(oss, value);
				bsonNode = new BsonInt64Node (value);
			}
				break;
			case BSON_BOOLEAN:
			{
				char value = this->ReadChar(oss);
				bsonNode = new BsonBoolNode(value == 1);
			}
				break;
			case BSON_STRING:
			{
				int size = 0;
				std::string value;
				this->ReadCommand(oss, size);
				this->ReadString(oss, value, size);
				bsonNode = new BsonStrNode(value);
			}
				break;
			case BSON_BINARY:
			{
				int size = 0;
				std::string value;
				this->ReadCommand(oss, size);
				value += this->ReadChar(oss);
				this->ReadString(oss, value, size);
				bsonNode = new BsonBinStrNode(value);
			}
				break;

			case BSON_DOCUMENT:
			{
				int size = 0;
				this->ReadCommand(oss, size);
				this->mMaxSize -= size;
				bsonNode = new BsonDocumentNode(oss, size);
			}
				break;
			case BSON_TIMESTAMP:
			{
				int inc, ts;
				this->ReadCommand(oss, inc);
				this->ReadCommand(oss, ts);
				long long value = (long long)inc << 32 | ts;
				bsonNode = new BsonCommandNode<long long, BSON_TIMESTAMP>(value);
			}
				break;
			case BSON_ARRAY:
			{
				int size = 0;
				this->ReadCommand(oss, size);
				this->mMaxSize -= size;
				bsonNode = new BsonArrayNode(oss, size);
			}
				break;
			case BSON_DATE:
			{
				long long data = 0;
				this->ReadCommand(oss, data);
				bsonNode = new BsonCommandNode<long long, BSON_DATE>(data);
			}
				break;
			case BSON_OBJECTID:
			{
				std::string value;
				this->ReadString(oss, value, 12);
				bsonNode = new BsonStringNode<BSON_OBJECTID>(value);
			}
				break;
			default:
				printf("type = %d\n", (int)bt);
				break;
			}
			bt = this->ReadChar(oss);
			this->AddNode(key, bsonNode);
		}

		return true;
	}
}

namespace Bson
{
	void BsonDecoder::Add(const std::string& key, int value)
	{
		this->AddNode(key, new BsonInt32Node(value));
	}

	void BsonDecoder::Add(const std::string& key, bool value)
	{
		this->AddNode(key, new BsonBoolNode (value));
	}

	void BsonDecoder::Add(const std::string& key, double value)
	{
		this->AddNode(key, new BsonDoubleNode (value));
	}

	void BsonDecoder::Add(const std::string& key, const char* value)
	{
		this->AddNode(key, new BsonStrNode(value));
	}

	void BsonDecoder::Add(const std::string& key, long long value)
	{
		this->AddNode(key, new BsonInt64Node(value));
	}

	void BsonDecoder::Add(const std::string& key, const std::string& value)
	{
		this->AddNode(key, new BsonStrNode(value));
	}

	void BsonDecoder::Add(const std::string& key, BsonNode* value)
	{
		this->AddNode(key, value);
	}
}

namespace Bson
{
	void BsonArrayNode::AddNode(const std::string& key, BsonNode* node)
	{
		this->mSize += 1;
		this->mSize += (key.size() + 1);
		this->mSize += node->GetSize();
		this->mNodes.emplace_back(node);
	}
	BsonArrayNode::BsonArrayNode(std::iostream& oss, int maxSize)
		: mSize(maxSize)
	{
		this->Decode(oss, maxSize);
	}

	void BsonArrayNode::Serialize(std::ostream& oss)
	{
		oss.write((char *)&this->mSize, sizeof(int));
		for(int index = 0; index < this->mNodes.size(); index++)
		{
			std::string key = std::to_string(index);
			printf("key = %s\n", key.c_str());
			oss << this->mNodes[index]->GetType() << key << '\0';
			this->mNodes[index]->Serialize(oss);
		}
	}
	const BsonNode* BsonArrayNode::GetNode(size_t index) const
	{
		if(index >= 0 && index < this->mNodes.size())
		{
			return this->mNodes[index];
		}
		return nullptr;
	}

	void BsonDocumentNode::AddNode(const std::string& key, BsonNode* node)
	{
		this->mSize += 1;
		this->mSize += (key.size() + 1);
		this->mSize += node->GetSize();
		this->mNodes.emplace(key, node);
	}

	BsonDocumentNode::BsonDocumentNode(std::iostream& oss, int size)
		: mSize(size)
	{
		this->Decode(oss, size);
	}

	void BsonDocumentNode::Serialize(std::ostream& oss)
	{
		auto iter = this->mNodes.begin();
		for(; iter != this->mNodes.end(); iter++)
		{
			printf("key = %s %p\n", iter->first.c_str(), this);
			BsonNode * bsonNode = iter->second;

			if(bsonNode->GetType() == BSON_DOCUMENT
				|| bsonNode->GetType() == BSON_ARRAY)
			{
				int size = bsonNode->GetSize();
				oss.write((char *)&size, sizeof(int));
			}
			oss << iter->second->GetType() << iter->first << '\0';
			iter->second->Serialize(oss);
		}
	}

	const BsonNode* BsonDocumentNode::GetNode(const std::string& key) const
	{
		auto iter = this->mNodes.find(key);
		return iter != this->mNodes.end() ? iter->second : nullptr;
	}

	void BsonDocumentNode::WriterToJson(std::string& json)
	{
		Json::Writer jsonWriter;
		auto iter = this->mNodes.begin();
		for(; iter != this->mNodes.end(); iter++)
		{
			jsonWriter.AddMember(iter->first);
			this->WriterToJson(iter->second, jsonWriter);
		}
		jsonWriter.WriterStream(json);
	}

	void BsonDocumentNode::WriterToJson(BsonArrayNode* bsonNode, Json::Writer& json)
	{
		json.StartArray();
		auto iter = bsonNode->Begin();
		for(; iter != bsonNode->End(); iter++)
		{
			this->WriterToJson(*iter, json);
		}
		json.EndArray();
	}

	void BsonDocumentNode::WriterToJson(BsonDocumentNode* bsonNode, Json::Writer& jsonWriter)
	{
		jsonWriter.StartObject();
		auto iter = bsonNode->mNodes.begin();
		for(; iter != bsonNode->mNodes.end(); iter++)
		{
			jsonWriter.AddMember(iter->first);
			this->WriterToJson(iter->second, jsonWriter);
		}
		jsonWriter.EndObject();
	}

	void BsonDocumentNode::WriterToJson(BsonNode * bsonNode, Json::Writer& jsonWriter)
	{
		switch (bsonNode->GetType())
		{
		case BSON_INT32:
			jsonWriter.AddMember(bsonNode->Cast<BsonInt32Node>()->GetData());
			break;
		case BSON_INT64:
			jsonWriter.AddMember(bsonNode->Cast<BsonInt64Node>()->GetData());
			break;
		case BSON_BOOLEAN:
			jsonWriter.AddMember(bsonNode->Cast<BsonBoolNode>()->GetData());
			break;
		case BSON_STRING:
			jsonWriter.AddMember(bsonNode->Cast<BsonStrNode>()->GetData());
			break;
		case BSON_BINARY:
			jsonWriter.AddMember(bsonNode->Cast<BsonBinStrNode>()->GetData());
			break;
		case BSON_REAL:
			jsonWriter.AddMember(bsonNode->Cast<BsonDoubleNode>()->GetData());
			break;
		case BSON_DATE:
			jsonWriter.AddMember(bsonNode->Cast<BsonCommandNode<long long, BSON_DATE>>()->GetData());
			break;
		case BSON_TIMESTAMP:
			jsonWriter.AddMember(bsonNode->Cast<BsonCommandNode<long long, BSON_TIMESTAMP>>()->GetData());
			break;
		case BSON_DOCUMENT:
			this->WriterToJson(bsonNode->Cast<BsonDocumentNode>(), jsonWriter);
			break;
		case BSON_ARRAY:
			this->WriterToJson(bsonNode->Cast<BsonArrayNode>(), jsonWriter);
			break;
		default:
			assert(false);
			break;
		}
	}
}