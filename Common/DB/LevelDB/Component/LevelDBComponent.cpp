//
// Created by yy on 2025/7/7.
//
#if __ENABLE_LEVEL_DB__
#include "LevelDBComponent.h"
namespace acs
{
    LevelDBComponent::LevelDBComponent()
    {
        this->mLevelDB = nullptr;
    }

    bool LevelDBComponent::LateAwake()
    {
        leveldb::Options openOptions;
        return leveldb::DB::Open(openOptions, "./level.db", &this->mLevelDB).ok();
    }

    bool LevelDBComponent::Set(const std::string& key, const std::string& value)
    {
        leveldb::Slice keySlice(key);
        leveldb::Slice valueSlice(value);
        return this->mLevelDB->Put(this->mWriteOptions, keySlice, valueSlice).ok();
    }

    bool LevelDBComponent::Set(const std::string& key, const json::w::Document& value)
    {
        size_t count = 0;
        std::unique_ptr<char> json;
        if(!value.Serialize(json, count))
        {
            return false;
        }
        leveldb::Slice keySlice(key);
        leveldb::Slice valueSlice(json.get(), count);
        return this->mLevelDB->Put(this->mWriteOptions, keySlice, valueSlice).ok();
    }

    bool LevelDBComponent::Get(const std::string& key, std::string& value)
    {
        leveldb::Slice keySlice(key);
        return this->mLevelDB->Get(this->mReadOptions, keySlice, &value).ok();
    }

    bool LevelDBComponent::Get(const std::string& key, json::r::Document& value)
    {
        std::string message;
        leveldb::Slice keySlice(key);
        if(!this->mLevelDB->Get(this->mReadOptions, keySlice, &message).ok())
        {
            return false;
        }
        return value.Decode(message);
    }
}
#endif