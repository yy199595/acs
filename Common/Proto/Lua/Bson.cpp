//
// Created by leyi on 2023/12/14.
//w

#include"Bson.h"
#include"Yyjson/Lua/ljson.h"
#include"Proto/Document/BsonDocument.h"

namespace lua
{

	bool LuaToBson(lua_State* L, int index, bson::w::Document& document);

	bool LuaArrayToBson(lua_State* L, int index, bson::w::Document& array);

	bool LuaToBson(lua_State* L, int index, bson::w::Document& document)
	{
		if (index < 0)
		{
			index = lua_gettop(L) + index + 1;
		}

		if (!lua_istable(L, index))
		{
			return false;
		}

		lua_pushnil(L);
		while (lua_next(L, index) != 0)
		{
			int keyIndex = -2;
			int valueIndex = -1;

			std::string key;
			if (lua_type(L, keyIndex) == LUA_TSTRING)
			{
				key = lua_tostring(L, keyIndex);
			}
			else if (lua_type(L, keyIndex) == LUA_TNUMBER)
			{
				key = std::to_string(lua_tointeger(L, keyIndex));
			}
			else
			{
				lua_pop(L, 1);
				continue;
			}

			switch (lua_type(L, valueIndex))
			{
				case LUA_TBOOLEAN:
				{
					bool value = lua_toboolean(L, valueIndex);
					document.Add(key.c_str(), value);
					break;
				}
				case LUA_TNUMBER:
				{
					if (lua_isinteger(L, valueIndex))
					{
						auto value = lua_tointeger(L, valueIndex);
						document.Add(key.c_str(), value);
					}
					else
					{
						double value = lua_tonumber(L, valueIndex);
						document.Add(key.c_str(), value);
					}
					break;
				}
				case LUA_TSTRING:
				{
					const char* value = lua_tostring(L, valueIndex);
					document.Add(key.c_str(), std::string(value));
					break;
				}
				case LUA_TTABLE:
				{
					if (lua_rawlen(L, valueIndex) > 0)
					{
						bson::w::Document array(false);
						if (LuaArrayToBson(L, valueIndex, array))
						{
							document.Add(key.c_str(), array);
						}
					}
					else
					{
						// 是嵌套表
						bson::w::Document subDocument;
						if (LuaToBson(L, valueIndex, subDocument))
						{
							document.Add(key.c_str(), subDocument);
						}
					}
					break;
				}
				case LUA_TNIL:
				{
					document.Add(key.c_str());
					break;
				}
				default:
				{
					break;
				}
			}

			lua_pop(L, 1);
		}

		return true;
	}

	bool LuaArrayToBson(lua_State* L, int index, bson::w::Document& array)
	{
		if (index < 0)
		{
			index = lua_gettop(L) + index + 1;
		}

		if (!lua_istable(L, index))
		{
			return false;
		}

		size_t length = lua_rawlen(L, index); // 获取数组长度
		for (size_t i = 1; i <= length; ++i)
		{
			lua_rawgeti(L, index, i); // 获取数组的第 i 个元素

			switch (lua_type(L, -1))
			{
				case LUA_TBOOLEAN:
				{
					array.Push(lua_toboolean(L, -1));
					break;
				}
				case LUA_TNUMBER:
				{
					if(lua_isinteger(L, index))
					{
						long long value = lua_tointeger(L, -1);
						if(value >= std::numeric_limits<int>::max())
						{
							array.Push(value);
						}
						else
						{
							array.Push((int)value);
						}
					}
					else
					{
						array.Push(lua_tonumber(L, -1));
					}
					break;
				}
				case LUA_TSTRING:
				{
					size_t count = 0;
					const char* value = luaL_tolstring(L, -1, &count);
					array.Push(std::string(value, count));
					break;
				}
				case LUA_TTABLE:
				{
					if (lua_rawlen(L, -1) > 0)
					{
						bson::w::Document subArray(false);
						if (LuaArrayToBson(L, -1, subArray))
						{
							array.Push(subArray);
						}
					}
					else
					{
						bson::w::Document subDocument;
						if (LuaToBson(L, -1, subDocument))
						{
							array.Push(subDocument);
						}
					}
					break;
				}
				default:
				{
					break;
				}
			}

			lua_pop(L, 1); // 弹出值
		}

		return true;
	}


	int lbson::encode(lua_State* L)
	{
		if (!lua_istable(L, 1))
		{
			luaL_error(L, "must type table");
			return 0;
		}
		bson::w::Document document;
		{
			int length = 0;
			LuaToBson(L, 1, document);
			const char* bson = document.Serialize(length);
			lua_pushlstring(L, bson, length);
		}
		return 1;
	}
}

namespace lua
{
	void BsonElementToLua(lua_State* L, const _bson::bsonelement& element);

	bool BsonToLua(lua_State* L, const _bson::bsonobj& document)
	{
		lua_newtable(L); // 创建一个新的 Lua 表

		// 遍历 BSON 对象中的字段
		_bson::bsonobjiterator bsonIterator(document);
		while (bsonIterator.more())
		{
			_bson::bsonelement element = bsonIterator.next();
			std::string key = element.fieldName();

			lua_pushstring(L, key.c_str());
			BsonElementToLua(L, element);
			lua_settable(L, -3);
		}

		return true;
	}

	void BsonElementToLua(lua_State* L, const _bson::bsonelement& element)
	{
		switch (element.type())
		{
			case _bson::BSONType::Bool:
			{
				lua_pushboolean(L, element.Bool());
				break;
			}
			case _bson::BSONType::NumberInt:
			{
				lua_pushinteger(L, element.Int());
				break;
			}
			case _bson::BSONType::NumberLong:
			{
				lua_pushinteger(L, element.Long());
				break;
			}
			case _bson::BSONType::NumberDouble:
			{
				lua_pushnumber(L, element.Double());
				break;
			}
			case _bson::BSONType::String:
			{
				lua_pushstring(L, element.String().c_str());
				break;
			}
			case _bson::BSONType::BinData:
			{
				int len = 0;
				const char* data = element.binData(len);
				lua_pushlstring(L, data, len); // 将二进制数据作为 Lua 字符串压入栈
				break;
			}
			case _bson::BSONType::Object:
			{
				BsonToLua(L, element.object()); // 递归处理嵌套对象
				break;
			}
			case _bson::BSONType::Array:
			{
				lua_newtable(L); // 创建一个新的 Lua 表表示数组

				std::vector<_bson::bsonelement> elements = element.Array();
				int index = 1; // Lua 数组从 1 开始索引
				for (const _bson::bsonelement& subElement: elements)
				{
					BsonElementToLua(L, subElement); // 将数组元素值压入栈
					lua_rawseti(L, -2, index++);    // 设置 Lua 表中的数组值
				}
				break;
			}
			case _bson::BSONType::Timestamp:
			{
				lua_pushinteger(L, static_cast<lua_Integer>(element.timestampValue()));
				break;
			}
			case _bson::BSONType::Date:
			{
				lua_pushinteger(L, static_cast<lua_Integer>(element.date().asInt64()));
				break;
			}
			case _bson::BSONType::jstOID:
			{
				lua_pushstring(L, element.__oid().toString().c_str());
				break;
			}
			default:
			{
				lua_pushstring(L, element.toString().c_str());
				break;
			}
		}
	}


	int lbson::decode(lua_State* L)
	{
		const char* bson = luaL_checkstring(L, 1);
		_bson::bsonobj document(bson);
		BsonToLua(L, document);
		return 1;
	}
}

namespace lua
{
	bool lbson::write(lua_State* L, const std::string& bson)
	{
		_bson::bsonobj document(bson.c_str());
		return BsonToLua(L, document);
	}

	bool lbson::read(lua_State* L, int index, std::string& bson)
	{
		bson::w::Document document;
		{
			int length = 0;
			if(!LuaToBson(L, index, document))
			{
				return false;
			}
			const char* str = document.Serialize(length);
			bson.assign(str, length);
		}
		return true;
	}
}