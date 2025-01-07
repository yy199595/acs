//
// Created by 64658 on 2025/1/2.
//

#include "WebSocketMessage.h"
#include "Util/Tools/Random.h"
namespace ws
{

	constexpr int CHAR_COUNT = std::numeric_limits<char>::max();
	constexpr int USHORT_COUNT = std::numeric_limits<unsigned short >::max();

	Message::Message()
	{
		this->Clear();
	}

	void Message::Clear()
	{
		this->mOffset = 0;
		this->mMessage.clear();
		memset(&this->mHeader, 0, sizeof(Header));
		memset(this->mMaskingKey, 0, sizeof(this->mMaskingKey));
	}

	int Message::OnSendMessage(std::ostream& os)
	{
		this->mHeader.mFin = true; // 完整帧
		this->mHeader.mRsv = 0;    // RSV 保持为 0
		//this->mHeader.mMask = false;  // 客户端消息 Mask 位应为 1，服务器发送时可为 0

		std::unique_ptr<char[]> buffer;

		if (this->mMessage.size() <= CHAR_COUNT - 2)
		{
			this->mHeader.mLength = this->mMessage.size(); // 直接使用消息大小
		}
		else if (this->mMessage.size() <= USHORT_COUNT)
		{
			this->mHeader.mLength = 126;
			buffer = std::make_unique<char[]>(2);  // 使用 2 个字节表示长度
			unsigned short size = this->mMessage.size();
			tcp::Data::Write<unsigned short >(buffer.get(), size);
		}
		else
		{
			this->mHeader.mLength = 127;
			buffer = std::make_unique<char[]>(8);  // 使用 8 个字节表示长度
			unsigned long long size = this->mMessage.size();
			tcp::Data::Write<unsigned long long>(buffer.get(), size);
		}

		// 构建第一个字节 (FIN + RSV + Opcode)
		unsigned char firstByte = 0x00;
		firstByte |= 0x80; // 设置 FIN 位
		firstByte |= (this->mHeader.mOpCode & 0x0F); // 设置操作码（例如 0x1 表示文本）

		os << firstByte;

		// 写入 Mask 位和消息长度
		unsigned char maskAndLengthByte = 0;
		if (this->mHeader.mMask)
		{
			maskAndLengthByte |= 0x80;
		}
		maskAndLengthByte |= this->mHeader.mLength;
		os << maskAndLengthByte;

		if (this->mHeader.mLength == 126)
		{
			os.write(buffer.get(), 2);
		}
		else if (this->mHeader.mLength == 127)
		{
			os.write(buffer.get(), 8);
		}
		if(this->mHeader.mMask)
		{
			os.write(this->mMaskingKey, 4);
		}
		os.write(this->mMessage.c_str(), this->mMessage.size());
		return 0;
	}

	void Message::SetBody(unsigned char opcode, const std::string& message, bool mask)
	{
		this->mHeader.mMask = mask;
		this->mHeader.mOpCode = opcode;
		this->mMessage.assign(message);
		if(this->mHeader.mMask)
		{
			for(size_t index = 0; index < sizeof(this->mMaskingKey); index++)
			{
				int num = help::Rand::Random<int>(0, sizeof(unsigned char));
				this->mMaskingKey[index] = static_cast<char>(num);
			}
			for(size_t index = 0; index < this->mMessage.size(); index++)
			{
				this->mMessage[index] = this->mMessage[index]  ^ this->mMaskingKey[index % 4];
			}
		}
	}

	int Message::OnRecvMessage(std::istream& os, size_t size)
	{
		if (this->mHeader.mLength == 0)
		{
			this->mOffset = 2;
			if (size < this->mOffset)
			{
				return tcp::ReadDecodeError;
			}
			int value = os.get();
			int value2 = os.get();
			this->mHeader.mFin = (value & 0x80) != 0;
			this->mHeader.mOpCode = value & 0x0F;
			this->mHeader.mMask = (value2 & 0x80) != 0;
			this->mHeader.mLength = value2 & 0x7F;
			if (!this->mHeader.mFin) //只支持完整包
			{
				return tcp::ReadDecodeError;
			}
			switch (this->mHeader.mLength)
			{
				case 126:
				{
					this->mOffset += 2;
					if (size < this->mOffset)
					{
						return tcp::ReadDecodeError;
					}
					char buffer[2] = { 0 };
					if(os.readsome(buffer, sizeof(buffer)) != 2)
					{
						return tcp::ReadDecodeError;
					}
					unsigned short len = 0;
					tcp::Data::Read(buffer, len);
					this->mHeader.mLength = len;
					break;
				}
				case 127:
				{
					this->mOffset += 8;
					if (size < this->mOffset)
					{
						return tcp::ReadDecodeError;
					}
					char buffer[8] = { 0 };
					os.readsome(buffer, sizeof(buffer));
					unsigned long long len = 0;
					tcp::Data::Read(buffer, len);
					this->mHeader.mLength = len;
					break;
				}
			}

			if (this->mHeader.mLength > ws::MESSAGE_MAX_COUNT)
			{
				return tcp::PacketLong;
			}

			if (this->mHeader.mMask)
			{
				this->mOffset += 4;
				if (size < this->mOffset)
				{
					return tcp::ReadDecodeError;
				}
				memset(this->mMaskingKey, 0, 4);
				os.readsome(this->mMaskingKey, 4);
			}
			this->mOffset = 0;
			this->mMessage.resize(this->mHeader.mLength);
		}

		int maxReadCount = this->mHeader.mLength - this->mOffset;
		char* buffer = const_cast<char*>(this->mMessage.c_str());
		size_t count = os.readsome(buffer, maxReadCount);
		while(count > 0)
		{
			this->mOffset += count;
			maxReadCount = this->mHeader.mLength - this->mOffset;
			count = os.readsome(buffer + this->mOffset, maxReadCount);
		}
		size_t len = this->mHeader.mLength - this->mOffset;
		if(len == 0)
		{
			if (this->mHeader.mMask)
			{
				for (size_t i = 0; i < this->mMessage.size(); ++i) {
					this->mMessage[i] ^= this->mMaskingKey[i % 4];
				}
			}
		}
		return tcp::ReadSomeMessage;
	}
}