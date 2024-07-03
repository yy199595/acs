//
// Created by leyi on 2024/2/4.
//

#ifndef APP_TIMECACHE_H
#define APP_TIMECACHE_H
#include <unordered_map>
#include "Util/Time/TimeHelper.h"
namespace custom
{
	template<typename K, typename V, int T>
	class TimeCache
	{
	public:
		bool Get(const K & k, V & v);
		void Set(const K & k, const V & v);
	private:
		std::unordered_map<K, std::tuple<long long, V>> mCache;
	};

	template<typename K, typename V, int T>
	inline bool TimeCache<K, V, T>::Get(const K& k, V & v)
	{
		auto iter = this->mCache.find(k);
		if(iter == this->mCache.end())
		{
			return false;
		}
		long long nowTime = help::Time::NowSec();
		if(nowTime >= std::get<0>(iter->second))
		{
			this->mCache.erase(iter);
			return false;
		}
		v = std::get<1>(iter->second);
		return true;
	}

	template<typename K, typename V, int T>
	void TimeCache<K, V, T>::Set(const K& k, const V& v)
	{
		long long nowTime = help::Time::NowSec();
		this->mCache[k] = std::make_tuple(nowTime + T, v);
	}

}

#endif //APP_TIMECACHE_H
