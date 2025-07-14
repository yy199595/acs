//
// Created by yy on 2025/7/7.
//

#pragma once

#if __ENABLE_LEVEL_DB__
#include "leveldb/db.h"
#include "Entity/Component/Component.h"
namespace acs
{
    class LevelDBComponent : public Component
    {
    public:
        LevelDBComponent();
    public:
        bool Set(const std::string & key, const std::string & value);
        bool Set(const std::string & key, const json::w::Document & value);
    public:
        bool Get(const std::string & key, std::string & value);
        bool Get(const std::string & key, json::r::Document & value);
    private:
        bool LateAwake() final;
    private:
        leveldb::DB * mLevelDB;
        leveldb::ReadOptions mReadOptions;
        leveldb::WriteOptions mWriteOptions;
    };
}


#endif
