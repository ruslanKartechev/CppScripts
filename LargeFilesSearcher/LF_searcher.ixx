#define WIN_PLATFORM

module;
#ifdef WIN_PLATFORM
	#include <windows.h>
#endif
export module LF_searcher;
import Logger;
import dir_utils;
import <iostream>;
import <string>;
import <memory>;
import <algorithm>;
import <vector>;
import <fstream>;

namespace SleepDev
{
	const int MaxConsoleLogLines = 128;

	export void BeginLFSearch();

	int _callsCount = 0;
	std::wstring _outputFilePathUser;
	std::wstring _userDir;
	uint32_t _maxSizeBytes = 50 * 1024 * 1024; // 50 MB Default
	

	class FileData;

	void TakeUserDirectory();
	void TakeUserMaxSize();
	void TakeUserOutputDirectory();
	bool AskIfOutputToDoc();
	bool AskIfFormatForLfs();
	void RunRecursiveSearch(std::wstring& globalDir, std::vector<FileData>* filesData);
	void LogFoundFilesToConsole(std::vector<FileData>* filesData);
	void WriteFoundFilesToDoc(std::vector<FileData>* filesData, bool formatForLfs);
	std::vector<FileData>* CreateFileDataVec();

	class FileData
	{
	public:
		/// <summary>
		/// name of the file including format
		/// </summary>
		std::wstring name;
		/// <summary>
		/// Path local to the given parent directory
		/// </summary>
		std::wstring localPath;
		std::uint32_t sizeBytes = 0;

		FileData(std::wstring path)
		{
			this->localPath = path;
		}

		FileData(std::wstring path, std::wstring name)
		{
			this->localPath = path;
			this->name = name;
		}

		FileData(std::wstring path, std::wstring name, uint32_t sizeBytes)
		{
			this->localPath = path;
			this->name = name;
			this->sizeBytes = sizeBytes;
		}

		~FileData() = default;

		std::uint32_t GetSizeKb()
		{
			return sizeBytes / 1024;
		}

		std::uint32_t GetSizeMb()
		{
			return sizeBytes / (1024 * 1024);
		}
	};

	export void BeginLFSearch()
	{
		TakeUserDirectory();
		TakeUserMaxSize();
		auto results = CreateFileDataVec();
		RunRecursiveSearch(_userDir, results);
		LogFoundFilesToConsole(results);
		if (AskIfOutputToDoc())
		{
			TakeUserOutputDirectory();
			auto lfsFormat = AskIfFormatForLfs();
			WriteFoundFilesToDoc(results, lfsFormat);
		}
		delete[] results;
	}

	std::vector<FileData>* CreateFileDataVec()
	{
		std::vector<FileData>* filesData = new std::vector<FileData>();
		filesData->reserve(100);
		return filesData;
	}

	void TakeUserDirectory()
	{
		bool proceed = false;
		char yesNoInput;
		do
		{
			LogLine("Input the parent directory for search: ");
			std::wcin >> _userDir;
			std::replace(_userDir.begin(), _userDir.end(), '/', '\\');
			LogLine(L"Your directory:  " + _userDir + L"   [y/n] ?");
			std::cin >> yesNoInput;
			if (yesNoInput == 'y')
				proceed = true;
			else
			{
				LogLine("Ok, start over");
				_userDir.clear();
				continue;
			}
			proceed = CheckFileExists(_userDir);
			if (!proceed)
				LogLine("Something is wrong with this path! ((");
			else
				LogLine("Path is Valid!");
		}
		while (!proceed);
	}

	void TakeUserMaxSize()
	{
		using namespace std;

		bool proceed = false;
		char yesNoInput;
		int numInput;
		int multiplier = 1;
		LogLine("Max size in Bytes [1], Kb [2] or Mb [3] ?");
		cin >> numInput;
		string sizeTypeStr = "Size is in: ";

		if (numInput >= 3)
		{
			sizeTypeStr = "Mb";
			multiplier = 1024 * 1024;
		}
		else if (numInput == 2)
		{
			sizeTypeStr = "Kb";
			multiplier = 1024;
		}
		else if (numInput <= 1)
		{
			sizeTypeStr = "Bytes";
		}
		LogLine("Enter Max size: ");
		cin >> numInput;
		string msg = "Max Size is: ";
		msg += to_string(numInput);
		msg += " " + sizeTypeStr;
		LogLine(msg);
		_maxSizeBytes = numInput * multiplier;
	}

	bool AskIfOutputToDoc()
	{
		LogLine("Write the results to file? [y/n]");
		char input;
		std::cin >> input;
		switch (input)
		{
		case 'y':
			return 1;
		default:
			return false;
		}
		return false;
	}

	bool AskIfFormatForLfs()
	{
		LogLine("Format for LFS? [y/n]");
		char input;
		std::cin >> input;
		switch (input)
		{
		case 'y':
			return 1;
		default:
			return false;
		}
		return false;
	}

	void TakeUserOutputDirectory()
	{
		bool proceed = false;
		char yesNoInput;
		do
		{
			LogLine("Output file directory: ");
			std::wcin >> _outputFilePathUser;
			std::replace(_outputFilePathUser.begin(), _outputFilePathUser.end(), '/', '\\');
			LogLine(L"Your directory:  " + _outputFilePathUser + L"   [y/n] ?");
			std::cin >> yesNoInput;
			if (yesNoInput == 'y')
				proceed = true;
			else
			{
				LogLine("Ok, start over");
				_outputFilePathUser.clear();
				continue;
			}
		}
		while (!proceed);
	}


#ifdef WIN_PLATFORM
	void RunRecursiveSearch_Windows(std::wstring&& parentDir, std::wstring&& localDir, std::vector<FileData>* filesData)
	{
		_callsCount++;
		if (_callsCount > 4096)
		{
			LogError("Calls count exceeded 4096!");
			return;
		}
		WIN32_FIND_DATA dataFind;
		HANDLE hFind;
		std::wstring parentPathAst = parentDir + std::wstring(L"\\*");
		hFind = FindFirstFile(parentPathAst.c_str(), &dataFind);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			//LogLine("Error! Invalid File Handle!");
			return;
		}
		// the first 2 are . and .. (IGNORE THEM)
		FindNextFile(hFind, &dataFind) != 0;
		FindNextFile(hFind, &dataFind) != 0;
		do
		{
			auto addedName = std::wstring(LR"(\)") + dataFind.cFileName;
			auto fSize = (dataFind.nFileSizeHigh * (MAXDWORD + 1)) + dataFind.nFileSizeLow;
			if ((dataFind.dwFileAttributes == CodeFile_Win) && fSize >= _maxSizeBytes)
			{
				filesData->push_back(FileData((localDir + addedName), dataFind.cFileName, fSize));
			}
			else if (dataFind.dwFileAttributes == CodeDirectory_Win)
			{
				std::wstring nLocalPath;
				if (localDir.length() == 0)
					nLocalPath = dataFind.cFileName;
				else
					nLocalPath = localDir + addedName;
				RunRecursiveSearch_Windows(parentDir + addedName, (std::wstring&&)nLocalPath, filesData);
			}
		}
		while (FindNextFile(hFind, &dataFind) != 0);
		FindClose(hFind);
	}
#endif


	void RunRecursiveSearch(std::wstring& globalDir, std::vector<FileData>* filesData)
	{
		_callsCount = 0;
#ifdef WIN_PLATFORM
		RunRecursiveSearch_Windows((std::wstring&&)globalDir, std::wstring(), filesData);
#endif
	}

	void LogFoundFilesToConsole(std::vector<FileData>* filesData)
	{
		LogLine("Found " + std::to_string(filesData->size()) + " files with size >= " + std::to_string(_maxSizeBytes / 1024) + "Kb");
		LogLine("Loggin found files' local path [no more than 128]");
		size_t counter = 0;
		size_t maxEntries = 128;
		for (auto it = filesData->begin(); it != filesData->end(); it++)
		{
			counter++;
			LogLine(it->localPath);
			if (counter >= maxEntries)
			{
				LogLine(std::string("TOO many entries >= ") + std::to_string(maxEntries));
				break;
			}
		}
	}

	void WriteStringsToDoc(std::vector<std::string>* strings)
	{
		LogLine(L"Writinge to " + _outputFilePathUser);
		std::ofstream outputFile;
		outputFile.open(_outputFilePathUser);
		if (!outputFile.is_open())
		{
			LogError("Failed to open or create the output file");
			return;
		}
		for (auto it = strings->begin(); it != strings->end(); it++)
		{
			outputFile << (it->c_str());
		}
		outputFile.close();
	}

	void WriteFoundFilesToDoc(std::vector<FileData>* filesData, bool formatForGitLfs)
	{
		if (filesData == nullptr)
		{
			LogError(R"(Passed "filesData" nullptr !!)");
			return;
		}
		std::vector<std::string>* outputStrings = new std::vector<std::string>();
		outputStrings->reserve(filesData->size());
		if (formatForGitLfs)
		{
			const auto str1 = std::string(" ");
			const auto str2 = std::string("[[:space:]]");
			for (auto it = filesData->begin(); it != filesData->end(); it++)
			{
				std::string temp = std::string((it->localPath).begin(), (it->localPath).end());
				ReplaceSubstrings(temp, str1, str2);
				temp += " filter=lfs diff=lfs merge=lfs -text\n";
				outputStrings->push_back(temp);
			}
		}
		else
		{
			for (auto it = filesData->begin(); it != filesData->end(); it++)
			{
				std::string temp = std::string((it->localPath).begin(), (it->localPath).end());
				outputStrings->push_back(temp);
			}
		}
		WriteStringsToDoc(outputStrings);
		delete[] outputStrings;
	}




}