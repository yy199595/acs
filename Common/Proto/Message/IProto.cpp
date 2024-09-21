#include"IProto.h"
#include"Util/Tools/Math.h"
#include"Util/Tools/TimeHelper.h"
#include"Yyjson/Document/Document.h"
#include"Lua/Engine/UserDataParameter.h"

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
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		return help::Math::ToNumber(iter->second, v);
	}

	bool IHeader::Has(const std::string& k) const
	{
		auto iter = this->mHeader.find(k);
		return iter != this->mHeader.end();
	}

	bool IHeader::Get(const std::string& k, long long& v) const
	{
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		return help::Math::ToNumber(iter->second, v);
	}

	bool IHeader::Get(const std::string& k, std::string& v) const
	{
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}

	bool IHeader::IsEqual(const std::string& k, const std::string& v) const
	{
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		return iter->second == v;
	}

	bool IHeader::Add(const std::string& k, int v)
	{
		auto iter = this->mHeader.find(k);
		if(iter != this->mHeader.end())
		{
			return false;
		}
		this->mHeader.emplace(k, std::to_string(v));
		return true;
	}

	bool IHeader::Add(const std::string& k, long long v)
	{
		auto iter = this->mHeader.find(k);
		if(iter != this->mHeader.end())
		{
			return false;
		}
		this->mHeader.emplace(k, std::to_string(v));
		return true;
	}

	void IHeader::WriteLua(lua_State* l, const tcp::IHeader& header)
	{
		Lua::UserDataParameter::UserDataStruct<const IHeader*>::Write(l, &header);
	}

	void IHeader::WriteLua(lua_State* L, const IHeader * header)
	{
		Lua::UserDataParameter::UserDataStruct<const IHeader*>::Write(L, header);
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
		long long number = 0;
		if (help::Math::ToNumber(value, number))
		{
			lua_pushinteger(L, number);
			return 1;
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
		std::string json;
		document.Encode(&json, true);
		lua_pushlstring(L, json.c_str(), json.size());
		return 1;
	}

	bool IHeader::Add(const std::string& k, const std::string& v)
	{
		auto iter = this->mHeader.find(k);
		if(iter != this->mHeader.end())
		{
			return false;
		}
		this->mHeader.emplace(k, v);
		return true;
	}

	void IHeader::Set(const std::string& k, const std::string& v)
	{
		this->mHeader[k] = v;
	}

	bool IHeader::Del(const std::string& k)
	{
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		this->mHeader.erase(iter);
		return true;
	}

	bool IHeader::Del(const std::string& k, int& v)
	{
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		if(!help::Math::ToNumber(iter->second, v))
		{
			return false;
		}
		this->mHeader.erase(iter);
		return true;
	}

	bool IHeader::Del(const std::string& k, std::string& v)
	{
		auto iter = this->mHeader.find(k);
		if(iter == this->mHeader.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}
}
