#include"Zip.h"
#include<memory>
#include"Core/Zip/miniz.h"
#include"Util/File/FileHelper.h"
namespace help
{
	bool zip::CompressFile(const std::string& path, std::string& output)
	{
		std::string input;
		if(!fs::ReadTxtFile(path, input))
		{
			return false;
		}
		return zip::CompressData(input.c_str(), input.size(), output);
	}

	bool zip::CompressData(const std::string& input, std::string& output)
	{
		return zip::CompressData(input.c_str(), input.size(), output);
	}

	bool zip::CompressData(const char* input, size_t size, std::string& output)
	{
		mz_ulong destLen = compressBound(size);
		std::unique_ptr<char[]> out(new char[destLen]);
		int status = mz_compress2((unsigned char*)&out[0], &destLen, (unsigned char*)input, size, MZ_BEST_COMPRESSION);
		if (status != MZ_OK) {
			return false;
		}
		output.assign((char *)out.get(), destLen);
		return true;
	}

	std::string zip::Compress(const std::string& input)
	{
		size_t size = input.size();
		const char * str = input.c_str();
		mz_ulong destLen = compressBound(input.size());
		std::unique_ptr<char[]> out(new char[destLen]);
		int status = mz_compress2((unsigned char*)&out[0], &destLen, (unsigned char*)str, size, MZ_BEST_COMPRESSION);
		if (status != MZ_OK) {
			return "";
		}
		return std::string(out.get(), destLen);
	}
}