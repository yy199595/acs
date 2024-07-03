//
// Created by leyi on 2023/8/11.
//

#ifndef APP_HASHMAP_H
#define APP_HASHMAP_H
#include<memory>
#include<unordered_map>
namespace custom
{
	template<typename Key, typename Val>
	class HashMap
	{
	public:
		inline Val Find(const Key & key);
		inline bool Has(const Key & key) const;
		inline bool Del(const Key & key);
		inline bool Del(const Key & key, Val & val);
		inline bool Get(const Key & key, Val & value);
		inline bool Add(const Key & key, const Val & val);
		inline void Set(const Key & key, const Val & val);
		inline const std::unordered_map<Key, Val> & Map() const { return this->mMap;}
	public:
		inline void Clear() { this->mMap.clear(); }
		inline size_t Size() const { return this->mMap.size();}
	public:
		inline auto End() const { return this->mMap.end();}
		inline auto Begin() const { return this->mMap.begin();}
	private:
		std::unordered_map<Key, Val> mMap;
	};

	template<typename Key, typename Val>
	Val HashMap<Key, Val>::Find(const Key& key)
	{
		auto iter = this->mMap.find(key);
		if(iter == this->mMap.end())
		{
			return nullptr;
		}
		return iter->second;
	}

	template<typename Key, typename Val>
	inline bool HashMap<Key, Val>::Del(const Key& key)
	{
		auto iter = this->mMap.find(key);
		if(iter == this->mMap.end())
		{
			return false;
		}
		this->mMap.erase(iter);
		return true;
	}

	template<typename Key, typename Val>
	inline bool HashMap<Key, Val>::Del(const Key& key, Val & val)
	{
		auto iter = this->mMap.find(key);
		if(iter == this->mMap.end())
		{
			return false;
		}
		val = std::move(iter->second);
		this->mMap.erase(iter);
		return true;
	}

	template<typename Key, typename Val>
	inline bool HashMap<Key, Val>::Has(const Key& key) const
	{
		auto iter = this->mMap.find(key);
		return iter != this->mMap.end();
	}

	template<typename Key, typename Val>
	inline bool HashMap<Key, Val>::Get(const Key& key, Val& value)
	{
		auto iter = this->mMap.find(key);
		if(iter == this->mMap.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}

	template<typename Key, typename Val>
	inline bool HashMap<Key, Val>::Add(const Key& key, const Val& val)
	{
		auto iter = this->mMap.find(key);
		if(iter != this->mMap.end())
		{
			return false;
		}
		this->mMap.emplace(key, val);
		return true;
	}

	template<typename Key, typename Val>
	inline void HashMap<Key, Val>::Set(const Key& key, const Val& val)
	{
		this->mMap[key] = val;
	}
}

#endif //APP_HASHMAP_H
