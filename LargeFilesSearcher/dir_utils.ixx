module;
#include <windows.h>
export module dir_utils;
import <string>;
import <vector>;
import <iostream>;

namespace SleepDev
{
	export const int CodeDirectory_Win = 16; // for windows
	export const int CodeFile_Win = 32; // for windows

	export struct FileInfo;

	export bool CheckFileExists(const std::wstring& path);
	export bool CheckDirExists(const std::wstring& path);
	export int CheckDirExistsCode(const std::wstring& path);
	export std::vector<std::wstring>* GetAllChildFilePaths(std::wstring parentDir);
	export std::vector<FileInfo>* GetAllChildInfo(std::wstring parentDir);
	export void GetDirectorySize(std::wstring parentDir, uint64_t& outSize);
	
	export uint32_t GetFullFileSize(WIN32_FIND_DATA& winData)
	{
		return (winData.nFileSizeHigh * (MAXDWORD + 1)) + winData.nFileSizeLow;
	}

	export struct FileInfo {
	public:
		std::wstring fullPath;
		std::wstring fullName;
		/// <summary>
		/// 1 is a File, 2 is Directory
		/// </summary>
		uint_fast8_t type; // directory or file etc.
		uint64_t sizeBytes;

		FileInfo() = default;

		~FileInfo() = default;

		FileInfo(WIN32_FIND_DATA winData)
		{
			sizeBytes = GetFullFileSize(winData);
			fullName = winData.cFileName;
			switch (winData.dwFileAttributes)
			{
			case CodeFile_Win:
				type = 1;
				break;
			case CodeDirectory_Win:
				type = 2;
				break;
			default:
				type = 3;
				break;
			}
		}
	};

	export void RunFilesListTest()
	{
		using namespace std;
		cout << "Enter input directory" << endl;
		wstring path;
		wcin >> path;
		auto files = GetAllChildFilePaths(path);
		if (files == nullptr)
		{
			std::cout << std::endl;
			std::cout << "CANNOT FIND FILES. NULL PTR" << std::endl;
			return;
		}
		for (std::wstring filePath : (*files))
		{
			wcout << (filePath) << endl;
		}
		auto filesData = GetAllChildInfo(path);
		for (auto f : *filesData)
		{
			cout << "---------------\n";
			wcout << L"File Path " << f.fullPath << std::endl;
			auto sizeMb = f.sizeBytes / (1024 * 1024);
			cout << "File Size " << to_string(sizeMb) << " Mb" << std::endl;
		}
	}

	export std::wstring SizeToMbStr(uint64_t sizeBytes)
	{
		return std::to_wstring((sizeBytes) / (1024 * 1024)) + std::wstring(L" MB");
	}

	export std::string SizeToMbStrA(uint64_t sizeBytes)
	{
		return std::to_string((sizeBytes) / (1024 * 1024)) + std::string(" MB");
	}

	/// <param name="path">The Full path to the file or dir</param>
	/// <returns>True if a something exists in path</returns>
	export bool CheckFileExists(const std::wstring& path)
	{
		auto str = std::string(path.begin(), path.end());
		DWORD ftyp = GetFileAttributesA(str.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!
		return true;    // this is not a directory!
	}

	/// <param name="path">The Full path to the directory</param>
	/// <returns>True if a Directory exists in path. If not a directory returns false</returns>
	export bool CheckDirExists(const std::wstring& path)
	{
		auto str = std::string(path.begin(), path.end());
		DWORD ftyp = GetFileAttributesA(str.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!
		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   // this is a directory!
		return false;    // this is not a directory!
	}

	export int CheckDirExistsCode(const std::wstring& path)
	{
		auto str = std::string(path.begin(), path.end());
		DWORD ftyp = GetFileAttributesA(str.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return 2;  //something is wrong with your path!
		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return 0;   // this is a directory!
		return 1;    // this is not a directory!
	}

	export std::vector<std::wstring>* GetAllChildFilePaths(std::wstring parentDir)
	{
		WIN32_FIND_DATA dataFind;
		HANDLE hFind;
		std::wstring parentPathAst = parentDir + std::wstring(L"\\*");
		hFind = FindFirstFile(parentPathAst.c_str(), &dataFind);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return nullptr;
		}
		// the first 2 are . and .. (IGNORE THEM)
		if (FindNextFile(hFind, &dataFind) == 0) // .
			return nullptr;
		if (FindNextFile(hFind, &dataFind) == 0) // ..
			return nullptr;
		auto vecPtr = new std::vector<std::wstring>();
		auto& dataVec = *vecPtr;
		dataVec.reserve(5);
		do
		{
			dataVec.push_back(std::wstring(parentDir) + std::wstring(LR"(\)") + dataFind.cFileName);

		}
		while (FindNextFile(hFind, &dataFind) != 0);

		return vecPtr;
	}

	export std::vector<FileInfo>* GetAllChildInfo(std::wstring parentDir)
	{
		WIN32_FIND_DATA dataFind;
		HANDLE hFind;
		std::wstring parentPathAst = parentDir + std::wstring(L"\\*");
		hFind = FindFirstFile(parentPathAst.c_str(), &dataFind);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return nullptr;
		}
		auto vecPtr = new std::vector<FileInfo>();
		auto& dataVec = *vecPtr;
		dataVec.reserve(5);
		// the first 2 are . and .. (IGNORE THEM)
		FindNextFile(hFind, &dataFind);
		FindNextFile(hFind, &dataFind);
		int iters = 0;
		int itersMax = 1024;
		uint64_t size = 0;
		do
		{
			iters++;
			if (iters >= itersMax)
			{
				std::cout << "ERROR! Did over 1024 iterations in the loop!" << std::endl;
				break;
			}
			auto info = FileInfo(dataFind);
			info.fullPath = std::wstring(parentDir) 
				+ std::wstring(LR"(\)") 
				+ dataFind.cFileName;
			if (dataFind.dwFileAttributes == CodeDirectory_Win)
			{
				size = 0;
				auto ptr = &size;
				GetDirectorySize(info.fullPath, size);
				info.sizeBytes = size;
			}
			dataVec.push_back(info);
		}
		while (FindNextFile(hFind, &dataFind) != 0);
		FindClose(hFind);
		return vecPtr;
	}


	/// <summary>
	/// Will Recursively go through all files in a folder to determine size
	/// </summary>
	/// <param name="parentDir">Parent Directory</param>
	/// <param name="outSize">Out Var. to store the total directory size</param>
	export void GetDirectorySize(std::wstring parentDir, uint64_t& outSize)
	{
		WIN32_FIND_DATA dataFind;
		HANDLE hFind;
		std::wstring parentPathAst = parentDir + std::wstring(L"\\*");
		hFind = FindFirstFile(parentPathAst.c_str(), &dataFind);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			std::cout << "ERROR !!! hFind == INVALID_HANDLE_VALUE" << std::endl;
			return;
		}

		// the first 2 are . and .. (IGNORE THEM)
		if (FindNextFile(hFind, &dataFind) == 0) // .
			return;
		if (FindNextFile(hFind, &dataFind) == 0) // ..
			return;
		// std::wcout << L"Stepped into: ";
		// std::wcout << parentDir << std::endl;
		// std::cout << "Total outSize on call: " << SizeToMbStrA(outSize) << std::endl;
		
		int iters = 0;
		int itersMax = 1024;
		do
		{
			iters++;
			if (iters >= itersMax)
			{
				std::cout << "ERROR! Did over 1024 iterations in the loop!" << std::endl;
				break;
			}
			if (dataFind.dwFileAttributes == CodeFile_Win)
			{
				outSize += GetFullFileSize(dataFind);
			}
			else if (dataFind.dwFileAttributes == CodeDirectory_Win)
			{
				auto path = std::wstring(parentDir)
					+ std::wstring(LR"(\)")
					+ dataFind.cFileName;
				GetDirectorySize(path, outSize);
			}
		}
		while (FindNextFile(hFind, &dataFind) != 0);
		FindClose(hFind);
	}

}