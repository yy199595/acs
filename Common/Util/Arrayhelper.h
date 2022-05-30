//
// Created by mac on 2022/5/27.
//

#ifndef SERVER_ARRAYHELPER_H
#define SERVER_ARRAYHELPER_H
#include<list>
#include<vector>
namespace Helper
{
	namespace Array
	{
		template<typename List, typename T>
		inline bool HasElement(const List & elments, const T & value)
		{
			for(const T & tmp : elments)
			{
				if(tmp == value)
				{
					return true;
				}
			}
			return false;
		}

		template<typename List1, typename List2, typename T>
		inline void AddRange(List1 & source, const List2 & target)
		{
			for(const T & tmp : target)
			{
				source.emplace_back(tmp);
			}
		}

		template<typename List,typename T>
		inline typename List::iterator Find(const List & source, const T & value)
		{
			auto iter = source.begin();
			for(; iter != source.end(); iter++)
			{
				if((*iter) == value)
				{
					return iter;
				}
			}
			return source.end();
		}

		template<typename List, typename T>
		inline bool Remove(List & source, const T & value)
		{
			auto iter = source.begin();
			for(; iter != source.end(); iter++)
			{
				if ((*iter) == value)
				{
					source.erase(iter);
					return true;
				}
			}
			return false;
		}
	}
}

#endif //SERVER_ARRAYHELPER_H
