#pragma once

#include"Yyjson/Src/yyjson.h"
#include"Lua/Engine/LuaInclude.h"

namespace lua
{
	const uint8_t max_encode_depth = 16;

	class JsonValue
	{
	public:
		~JsonValue() { yyjson_mut_doc_free(this->doc);}
	public:
		yyjson_mut_doc * doc;
		yyjson_mut_val * val;
	};

	class yyjson
	{
	public:
		static int read_file(lua_State * L);

		static int encode(lua_State* L);

		static int pretty(lua_State* L);


		static int encode_impl(lua_State* L, yyjson_write_flag flag, bool emy_as_arr);

		static bool read(lua_State* L, int inx, std::string& json);

		static bool read(lua_State* L, int inx, JsonValue & jsonValue);

		static int decode(lua_State* L);

		static bool write(lua_State* L, const char* buf, size_t len, bool numkeyable = false);

		static bool write(lua_State* L, yyjson_doc * val, bool numkeyable = false);

		static bool write(lua_State* L, yyjson_val* val, bool numkeyable = false);

	protected:
		static bool is_array(lua_State* L, int index, bool emy_as_arr);

		static yyjson_mut_val* encode_one(lua_State* L, yyjson_mut_doc* doc, bool emy_as_arr, int idx, int depth);

		static yyjson_mut_val* key_encode(lua_State* L, yyjson_mut_doc* doc, int idx);

		static yyjson_mut_val* array_encode(lua_State* L, yyjson_mut_doc* doc, bool emy_as_arr, int index, int depth);

		static yyjson_mut_val* table_encode(lua_State* L, yyjson_mut_doc* doc, bool emy_as_arr, int index, int depth);

		static void number_decode(lua_State* L, yyjson_val* val);

		static void array_decode(lua_State* L, yyjson_val* val, bool numkeyable);

		static void table_decode(lua_State* L, yyjson_val* val, bool numkeyable);

		static void decode_one(lua_State* L, yyjson_val* val, bool numkeyable);
	};
}
