//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_DATAPOOL_H
#define APP_DATAPOOL_H

#include<list>
#include<string>
#include<memory>
#include<unordered_map>
namespace Pool
{
	template<typename K, typename V>
	class LRUCache {
	public:
		LRUCache(int capacity) {
			this->capacity = capacity;
		}

		std::shared_ptr<V> Get(K key) {
			auto it = cache.find(key);
			if (it == cache.end()) {
				// Key not found in cache
				return nullptr;
			}

			cacheList.splice(cacheList.begin(), cacheList, it->second);
			return it->second->second;
		}

		void Put(K key, std::shared_ptr<V> value) {
			auto it = cache.find(key);
			if (it != cache.end()) {
				it->second->second = value;
				cacheList.splice(cacheList.begin(), cacheList, it->second);
				return;
			}

			cacheList.emplace_front(key, value);
			cache[key] = cacheList.begin();

			if (cache.size() > capacity) {
				K keyToRemove = cacheList.back().first;
				cacheList.pop_back();
				cache.erase(keyToRemove);
			}
		}

	private:
		int capacity;
		std::list<std::pair<K, std::shared_ptr<V>>> cacheList;
		std::unordered_map<K, typename std::list<std::pair<K, std::shared_ptr<V>>>::iterator> cache;
	};
}



#endif //APP_DATAPOOL_H
