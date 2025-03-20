#pragma once
#include <iostream>
#include <unordered_map>
#include <list>
#include <memory>

namespace custom
{
	template<typename KeyType, typename ValueType>
	class LRUCache
	{
	public:
		explicit LRUCache(size_t capacity) : capacity_(capacity)
		{
		}

		// 获取缓存中的值
		ValueType* Get(const KeyType& key)
		{
			auto it = cache_.find(key);
			if (it != cache_.end())
			{
				// 将访问的元素移到LRU列表的头部
				lruList_.splice(lruList_.begin(), lruList_, it->second);
				return it->second->second.get();
			}
			return nullptr;  // 返回空指针表示未找到
		}

		// 插入键值对到缓存
		void Set(const KeyType& key, std::unique_ptr<ValueType> value)
		{
			auto it = cache_.find(key);
			if (it != cache_.end())
			{
				// 如果键已经存在，更新值，并将元素移到LRU列表的头部
				it->second->second = std::move(value);
				lruList_.splice(lruList_.begin(), lruList_, it->second);
			}
			else
			{
				// 如果缓存已满，移除LRU列表尾部的元素
				if (cache_.size() >= capacity_)
				{
					const auto& lastKey = lruList_.back().first;
					cache_.erase(lastKey);
					lruList_.pop_back();
				}

				// 插入新元素到缓存和LRU列表的头部
				lruList_.emplace_front(key, std::move(value));
				cache_[key] = lruList_.begin();
			}
		}

		// 手动移除缓存中的元素
		bool Del(const KeyType& key)
		{
			auto it = cache_.find(key);
			if (it != cache_.end())
			{
				lruList_.erase(it->second); // 从LRU列表中移除
				cache_.erase(key); // 从缓存中移除
				return true;
			}
			return false;
		}

	private:
		size_t capacity_;
		std::list<std::pair<KeyType, std::unique_ptr<ValueType>>> lruList_;
		std::unordered_map<KeyType, typename std::list<std::pair<KeyType, std::unique_ptr<ValueType>>>::iterator> cache_;
	};
}

