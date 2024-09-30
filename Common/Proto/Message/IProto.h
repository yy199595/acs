//
// Created by yjz on 2022/5/18.
//

#ifndef APP_IPROTO_H
#define APP_IPROTO_H
#include<ostream>
#include<istream>
#include<cstring>
#include<unordered_map>
struct lua_State;

namespace tcp
{

	constexpr int ReadDone = 0;
	constexpr int ReadOneLine = -1; //读一行
	constexpr int ReadSomeMessage = -2; //读一些
	constexpr int ReadError = -3;    //读取错误
	constexpr int ReadDecodeError = -4; //解析错误
    constexpr int ReadPause = -5; //暂停
	constexpr int PacketLong = -6; //包体过长
	constexpr int ReadAll = -7;

	namespace Decode
	{
		constexpr int None = 0;
		constexpr int ProtoHead = 1;
		constexpr int MessageHead = 2;
		constexpr int MessageBody = 3;
		constexpr int Done = 4;
	}

	class IProto
	{
	public:
		IProto();

		virtual ~IProto() = default;

	public:
		IProto(IProto&&) = delete;

		IProto(const IProto&) = delete;

		IProto(const IProto&&) = delete;

		IProto& operator=(const IProto&) = delete;

		long long GetCostTime() const;
	public:
		virtual void Clear() = 0;

		virtual std::string ToString()
		{
			return {};
		}

		virtual int OnSendMessage(std::ostream& os) = 0; //返回剩余要发送的字节数
		virtual int OnRecvMessage(std::istream& os, size_t size)
		{
			return 0;
		};
	private:
		long long mStartTime;
	};

	namespace Data
	{
		template<typename T>
		union Value
		{
		public:
			T Data;
			char Buffer[sizeof(T)] = { 0 };
		};

		template<typename T>
		inline void Read(std::istream& os, T& value)
		{
			char byteArr[sizeof(T)] = { 0 };
			os.readsome(byteArr, sizeof(T));
			memcpy(&value, byteArr, sizeof(T));
		}

		template<typename T>
		inline void Write(std::ostream& is, const T value)
		{
			char byteArr[sizeof(T)] = { 0 };
			memcpy(byteArr, &value, sizeof(T));
			is.write(byteArr, sizeof(T));
		}

		inline void Write(std::ostream& os, const std::string& value)
		{
			if (!value.empty())
			{
				os.write(value.c_str(), value.size());
			}
		}

		inline void Write(std::ostream& os, const char* str, size_t size)
		{
			if (str != nullptr && size > 0)
			{
				os.write(str, size);
			}
		}
	}

	class IHeader
	{
	public:
		IHeader() = default;
		virtual ~IHeader() = default;
	public:
		bool Add(const std::string & k, int v);
		bool Add(const std::string & k, long long v);
		bool Add(const std::string & k, const std::string & v);
		void Set(const std::string & k, const std::string & v);
	public:
		static int LuaGet(lua_State * l);
		static int LuaToString(lua_State * l);
		static void WriteLua(lua_State *l, const IHeader & header);
		static void WriteLua(lua_State *l, const IHeader * header);
	public:
		auto End() const { return this->mHeader.end(); }
		auto Begin() const { return this->mHeader.begin(); }
        size_t Count() const { return this->mHeader.size(); }
	public:
		bool Del(const std::string& k);
		bool Del(const std::string& k, int & v);
		bool Del(const std::string& k, std::string & v);
	public:
		bool Has(const std::string &k) const;
		bool Get(const std::string &k, int & v) const;
		bool Get(const std::string &k, long long & v) const;
		bool Get(const std::string &k, std::string & v) const;
		bool IsEqual(const std::string &k, const std::string & v) const;
	protected:
		std::unordered_map<std::string, std::string> mHeader;
	};
}
#endif //APP_IPROTO_H
