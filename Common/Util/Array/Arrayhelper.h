//
// Created by mac on 2022/5/27.
//

#ifndef APP_ARRAYHELPER_H
#define APP_ARRAYHELPER_H
#include<list>
#include<vector>
#include<string>
namespace help
{
	namespace Array
	{
		inline std::string Concat(const std::vector<std::string> & list, const char c)
		{
			if(list.size() == 1)
			{
				return list[0];
			}
			else
			{
				std::string result;
				for(size_t index = 0; index < list.size(); index++)
				{
					if(index == list.size() - 1)
					{
						result.append(list[index]);
					}
					else
					{
						result.append(list[index]);
						result += c;
					}
				}
				return result;
			}
		}
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
			for(; iter != source.end(); ++iter)
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
			for(; iter != source.end(); ++iter)
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

#endif //APP_ARRAYHELPER_H
