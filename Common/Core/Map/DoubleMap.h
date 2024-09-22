//
// Created by 64658 on 2024/9/3.
//

#ifndef APP_DOUBLEMAP_H
#define APP_DOUBLEMAP_H
#include "unordered_map"
namespace custom
{
	template<typename K, typename V>
	class DoubleMap
	{
	public:
		void DeleteByKey(const K & k);
		void DeleteByValue(const V & v);
		void Add(const K & k, const V & v);
	public:
		bool FindByKey(const K & k, V & v);
		bool FindByValue(const V & v, K & k);
	private:
		std::unordered_map<K, V> mMap1;
		std::unordered_map<V, K> mMap2;
	};
	template<typename K, typename V>
	inline void DoubleMap<K, V>::Add(const K& k, const V& v)
	{
		this->mMap1.emplace(k, v);
		this->mMap2.emplace(v, k);
	}
	template<typename K, typename V>
	inline void DoubleMap<K, V>::DeleteByKey(const K& k)
	{
		auto iter1 = this->mMap1.find(k);
		if(iter1 == this->mMap1.end())
		{
			return;
		}
		auto iter2 = this->mMap2.find(iter1->second);
		if(iter2 != this->mMap2.end())
		{
			this->mMap2.erase(iter2);
		}
		this->mMap1.erase(iter1);
	}

	template<typename K, typename V>
	inline void DoubleMap<K, V>::DeleteByValue(const V& v)
	{
		auto iter1 = this->mMap2.find(v);
		if(iter1 == this->mMap2.end())
		{
			return;
		}
		auto iter2 = this->mMap1.find(iter1->second);
		if(iter2 != this->mMap1.end())
		{
			this->mMap1.erase(iter2);
		}
		this->mMap2.erase(iter1);
	}

	template<typename K, typename V>
	inline bool DoubleMap<K, V>::FindByKey(const K& k, V& v)
	{
		auto iter = this->mMap1.find(k);
		if(iter == this->mMap1.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}

	template<typename K, typename V>
	inline bool DoubleMap<K, V>::FindByValue(const V& v, K& k)
	{
		auto iter = this->mMap2.find(k);
		if(iter == this->mMap2.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}
}

#endif //APP_DOUBLEMAP_H
