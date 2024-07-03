//
// Created by leyi on 2023/8/17.
//

#ifndef APP_HASHSET_H
#define APP_HASHSET_H
#include<unordered_set>

namespace custom
{
	template<typename T>
	class HashSet
	{
	public:
		inline bool Del(const T & key);
		inline bool Add(const T & key);
		inline bool Has(const T & key) const;
	private:
		std::unordered_set<T> mSet;
	};

	template<typename T>
	inline bool HashSet<T>::Del(const T& key)
	{
		auto iter = this->mSet.find(key);
		if(iter == this->mSet.end())
		{
			return false;
		}
		this->mSet.erase(iter);
		return true;
	}
	template<typename T>
	inline bool HashSet<T>::Has(const T& key) const
	{
		auto iter = this->mSet.find(key);
		return iter != this->mSet.end();
	}
	template<typename T>
	bool HashSet<T>::Add(const T& value)
	{
		auto iter = this->mSet.find(value);
		if(iter != this->mSet.end())
		{
			return false;
		}
		this->mSet.emplace(value);
		return true;
	}
}
#endif //APP_HASHSET_H
