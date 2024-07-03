//
// Created by leyi on 2024/1/19.
//

#ifndef APP_CACHEDATA_H
#define APP_CACHEDATA_H

namespace cache
{
	template<typename T>
	struct Data
	{
	public:
		T data;
		long long time;
	};

}


#endif //APP_CACHEDATA_H
