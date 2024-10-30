#ifndef APP_BINDSTREAM_H
#define APP_BINDSTREAM_H

#include <string>
#include <cstring>

namespace help
{
	class BindStream
	{
	public:
		BindStream(char * msg, size_t len)
				: mMessage(msg), mLength(len), mPos(0)
		{

		}

	public:
		template<typename T>
		inline bool Read(T & value);
		inline bool GetLine(std::string & line);
		inline size_t ReadAll(std::string & buffer);
		inline size_t ReadSome(char * buffer, size_t count);

		inline size_t GetPos() const { return this->mPos; }

		template<typename T>
		inline bool Write(const T & value);
		inline bool Write(const std::string & value);
		inline bool Write(const char * value, size_t size);

	private:
		size_t mPos;        // 当前读取位置
		size_t mLength;     // 数据总长度
		char * mMessage;    // 数据源
	};

	template<typename T>
	inline bool BindStream::Read(T& value)
	{
		constexpr size_t len = sizeof(T);
		if (this->mPos + len > this->mLength)
		{
			return false; // 超出长度
		}
		memcpy(&value, this->mMessage + this->mPos, len);
		this->mPos += len; // 更新读取位置
		return true; // 成功读取
	}

	inline size_t BindStream::ReadSome(char* buffer, size_t count)
	{
		if (buffer == nullptr || count == 0) {
			return 0; // 不合法的参数
		}

		size_t available = mLength - mPos;
		if (count > available) {
			count = available; // 调整为可读取的字节数
		}
		memcpy(buffer, mMessage + mPos, count);
		mPos += count; // 更新当前位置
		return count; // 返回实际读取的字节数
	}

	inline size_t BindStream::ReadAll(std::string& buffer)
	{
		if (mPos >= mLength) {
			return 0; // 没有可读取的数据
		}

		size_t len = mLength - mPos;
		buffer.assign(mMessage + mPos, len); // 读取所有剩余数据
		mPos = mLength; // 更新到末尾
		return len; // 返回读取的字节数
	}

	inline bool BindStream::GetLine(std::string& line)
	{
		const char* msg = mMessage + mPos;
		const char* str = strchr(msg, '\n');

		if (str == nullptr) {
			if (mPos < mLength) {
				// 返回剩余的数据作为最后一行
				line.assign(msg, mLength - mPos);
				mPos = mLength; // 更新位置到末尾
				return true; // 成功返回最后一行
			}
			return false; // 没有更多数据
		}

		size_t count = str - msg;
		if (count > 0) {
			line.assign(msg, count); // 读取当前行
		}
		mPos += count + 1; // 更新位置
		return true; // 成功读取一行
	}

	template<typename T>
	bool BindStream::Write(const T& value)
	{
		constexpr size_t len = sizeof(value);
		if (this->mPos + len > this->mLength)
		{
			return false; // 超出长度
		}
		memcpy(this->mMessage + this->mPos, &value, len);
		this->mPos += len; // 更新位置
		return true; // 成功写入
	}

	bool BindStream::Write(const std::string& value)
	{
		if (this->mPos + value.size() > this->mLength)
		{
			return false; // 超出长度
		}
		memcpy(this->mMessage + this->mPos, value.c_str(), value.size());
		this->mPos += value.size(); // 更新位置
		return true; // 成功写入
	}

	bool BindStream::Write(const char* value, size_t size)
	{
		if (value == nullptr || this->mPos + size > this->mLength)
		{
			return false; // 不合法的参数或超出长度
		}
		memcpy(this->mMessage + this->mPos, value, size);
		this->mPos += size; // 更新位置
		return true; // 成功写入
	}
}

#endif // APP_BINDSTREAM_H
