//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_DATAPOOL_H
#define APP_DATAPOOL_H

#include<list>
#include<string>
#include<memory>
#include<unordered_map>
namespace pool
{
    template<typename TKey, typename TValue>
    class DataPool
    {
    public:
        DataPool(size_t count):mMaxCount(count) { }
    public:
        void Clear();
        bool Remove(const TKey & key);
        std::shared_ptr<TValue> Get(const TKey & kye);
        void Add(const TKey & key, std::shared_ptr<TValue> value);
    private:
        bool RemoveKey(const TKey & key);
    private:
        std::list<TKey> mKeys;
        const size_t mMaxCount;
        std::unordered_map<TKey, std::shared_ptr<TValue>> mDatas;
    };
    template<typename TKey, typename TValue>
    void DataPool<TKey, TValue>::Clear()
    {
        this->mKeys.clear();
        this->mDatas.clear();
    }

    template<typename TKey, typename TValue>
    bool DataPool<TKey, TValue>::Remove(const TKey &key)
    {
        auto iter = this->mDatas.find(key);
        if(iter != this->mDatas.end())
        {
            this->RemoveKey(key);
            this->mDatas.erase(iter);
            return true;
        }
        return false;
    }

    template<typename TKey, typename TValue>
    bool DataPool<TKey, TValue>::RemoveKey(const TKey &key)
    {
        auto iter = this->mKeys.begin();
        for (; iter != this->mKeys.end(); iter++)
        {
            if ((*iter) == key)
            {
                this->mKeys.erase(iter);
                return true;
            }
        }
        return false;
    }

    template<typename TKey, typename TValue>
    void DataPool<TKey, TValue>::Add(const TKey &key, std::shared_ptr<TValue> value)
    {
        this->RemoveKey(key);
        this->mDatas[key] = value;
        this->mKeys.emplace_back(key);
        while (!this->mKeys.empty() && this->mDatas.size() >= this->mMaxCount)
        {
            TKey id = this->mKeys.front();
            auto iter1 = this->mDatas.find(id);
            if (iter1 != this->mDatas.end())
            {
                this->mDatas.erase(iter1);
            }
            this->mKeys.pop_front();
        }
    }
    template<typename TKey, typename TValue>
    std::shared_ptr<TValue> DataPool<TKey, TValue>::Get(const TKey &key)
    {
        auto iter = this->mDatas.find(key);
        if(iter != this->mDatas.end())
        {
            this->RemoveKey(key);
            this->mKeys.emplace_back(key);
            return iter->second;
        }
        return nullptr;
    }
}


#endif //APP_DATAPOOL_H
