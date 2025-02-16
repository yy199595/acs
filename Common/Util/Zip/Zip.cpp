//
// Created by yy on 2025/2/5.
//

#include "Zip.h"
#include "Core/Excel/miniz.h"
#include "Util/File/DirectoryHelper.h"
namespace help
{
	bool zip::Unzip(const std::string& output_dir, const std::string& zip_path)
	{
		if(!help::dir::DirectorIsExist(output_dir))
		{
			help::dir::MakeDir(output_dir);
		}
		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(zip_archive));
		if (!mz_zip_reader_init_file(&zip_archive, zip_path.c_str(), 0)) {
			return false;
		}

		int num_files = mz_zip_reader_get_num_files(&zip_archive);
		if (num_files == 0) {
			mz_zip_reader_end(&zip_archive);
			return false;
		}

		for (int i = 0; i < num_files; ++i) {
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
				return false;
			}

			std::string file_name = file_stat.m_filename;
			std::string output_path = output_dir + "/" + file_name;

			if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
				help::dir::MakeDir(file_name);
			}
			if (!mz_zip_reader_extract_to_file(&zip_archive, i, output_path.c_str(), 0)) {
				return false;
			}
		}
		mz_zip_reader_end(&zip_archive);
		return true;
	}

    bool zip::Create(const std::string & dir, const std::string & path)
    {
        std::vector<std::string> files;
        if(!help::dir::GetFilePaths(dir, files))
        {
            return false;
        }
		return zip::Create(dir, files, path);
    }

	bool zip::Create(const std::string& dir, const std::vector<std::string>& files, const std::string& path)
	{
		mz_zip_archive zip_archive;
		int pos = dir.back() == '/' ? 0 : 1;
		memset(&zip_archive, 0, sizeof(zip_archive));
		if(!mz_zip_writer_init_file(&zip_archive, path.c_str(), 0))
		{
			return false;
		}
		for(const std::string & filePath : files)
		{
			if(help::dir::IsDir(filePath))
			{
				std::vector<std::string> tempFiles;
				help::dir::GetFilePaths(filePath, tempFiles);
				for(const std::string & path : tempFiles)
				{
					std::string relativePath = path.substr(dir.size() + pos);
					if(!mz_zip_writer_add_file(&zip_archive, relativePath.c_str(),
							path.c_str(), nullptr, 0, MZ_DEFAULT_COMPRESSION))
					{
						mz_zip_writer_end(&zip_archive); // 清理资源
						return false;
					}
				}
			}
			else
			{
				std::string relativePath = filePath.substr(dir.size() + pos);
				if(!mz_zip_writer_add_file(&zip_archive, relativePath.c_str(),
						filePath.c_str(), nullptr, 0, MZ_DEFAULT_COMPRESSION))
				{
					mz_zip_writer_end(&zip_archive); // 清理资源
					return false;
				}
			}
		}
		mz_zip_writer_finalize_archive(&zip_archive);
		mz_zip_writer_end(&zip_archive);
		return true;
	}
}