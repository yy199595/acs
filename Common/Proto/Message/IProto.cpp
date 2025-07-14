#include "IProto.h"
#include <algorithm>
#include "Util/Tools/Math.h"
#include "Util/Tools/TimeHelper.h"
#include "Yyjson/Document/Document.h"
#include "Lua/Engine/UserDataParameter.h"

namespace tcp
{
	IProto::IProto()
		: mStartTime(help::Time::NowMil())
	{

	}

	long long IProto::GetCostTime() const
	{
		return help::Time::NowMil() - this->mStartTime;
	}
}

namespace tcp
{
	bool IHeader::Get(const std::string& k, int& v) const
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		return help::Math::ToNumber(iter->second, v);
	}

	bool IHeader::Has(const std::string& k) const
	{
		return std::any_of(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
	}

	bool IHeader::Get(const std::string& k, long long& v) const
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		return help::Math::ToNumber(iter->second, v);
	}

	bool IHeader::Get(const std::string& k, std::string& v) const
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}


	bool IHeader::IsEqual(const std::string& k, const std::string& v) const
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		return iter->second == v;
	}

	void IHeader::Add(const std::string& k, int v)
	{
		this->mHeader.emplace_back(k, std::to_string(v));
	}

	void IHeader::Add(const std::string& k, long long v)
	{

		this->mHeader.emplace_back(k, std::to_string(v));
	}

	void IHeader::WriteLua(lua_State* l, const tcp::IHeader& header)
	{
		Lua::UserDataParameter::UserDataStruct<const IHeader*>::Write(l, &header);
	}

	int IHeader::LuaGet(lua_State* L)
	{
		IHeader* head = Lua::UserDataParameter::Read<IHeader*>(L, 1);
		const char* key = luaL_checkstring(L, 2);
		if (head == nullptr || key == nullptr)
		{
			return 0;
		}
		std::string value;
		if (!head->Get(key, value))
		{
			return 0;
		}
		lua_pushlstring(L, value.c_str(), value.size());
		return 1;
	}

	int IHeader::LuaToString(lua_State* L)
	{
		IHeader* head = Lua::UserDataParameter::Read<IHeader*>(L, 1);
		if(head == nullptr)
		{
			return 0;
		}
		json::w::Document document;
		for(auto iter = head->Begin(); iter != head->End(); iter++)
		{
			document.Add(iter->first.c_str(), iter->second);
		}
		size_t count = 0;
		std::unique_ptr<char> json;
		if(!document.Serialize(json, count))
		{
			return 0;
		}
		lua_pushlstring(L, json.get(), count);
		return 1;
	}

	void IHeader::Add(const std::string& k, const std::string& v)
	{
		if(k.empty() || v.empty())
		{
			return;
		}
		this->mHeader.emplace_back(k, v);
	}

	void IHeader::Set(const std::string& k, const std::string& v)
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter != this->mHeader.end())
		{
			iter->second = v;
			return;
		}
		this->mHeader.emplace_back(k, v);
	}

	bool IHeader::Del(const std::string& k)
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		this->mHeader.erase(iter);
		return true;
	}

	bool IHeader::Del(const std::string& k, int& v)
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		if (!help::Math::ToNumber(iter->second, v))
		{
			return false;
		}
		this->mHeader.erase(iter);
		return true;
	}

	bool IHeader::Del(const std::string& k, std::string& v)
	{
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&k](const std::pair<std::string, std::string>& item)
				{
					return item.first == k;
				});
		if (iter == this->mHeader.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}
}
