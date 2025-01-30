module;
#include <windows.h>

export module push_helper;
import dir_utils;
import Logger;
import <iostream>;
import <string>;
import <memory>;
import <algorithm>;
import <vector>;
import <fstream>;

namespace SleepDev
{
	class FilesGroup;

	export void BeginPushHelper();
	export void WritePushGroupsToFile(std::vector<FilesGroup*>& groups, std::wstring parentDir, std::wstring outputDir);
	export std::vector<FilesGroup*> MakePushGroups(std::wstring parentPath);

	void TakeUserDirectory();
	std::wstring TakeUserOutputDirectory();
	void CreatePushGroupsRecursive(std::wstring mainDirPath, std::vector<FilesGroup*>& outGroups, bool checkOwnSize);

	
	const uint32_t MAX_PUSH_SIZE = 1  * 1024 * 1024 * 1024; // 1 GB
	std::wstring _userDir;

	class FilesGroup {
	public:
		std::vector<std::wstring> localPaths; // Could be both a directory and a file
		uint32_t totalSize;
		FilesGroup()
		{
			totalSize = 0;
			localPaths = std::vector<std::wstring>();
			localPaths.reserve(5);
		}

		~FilesGroup()
		{
			localPaths.clear();
		}
	};

	export void BeginPushHelper()
	{
		using namespace std;
		LogLine("Started Push Groups helper");
		TakeUserDirectory();
		wstring assetsPath = _userDir + LR"(\Assets)";
		LogLine(std::wstring(L"Looking inside ") + assetsPath);
		
		auto groups = MakePushGroups(assetsPath);

		auto outputDir = TakeUserOutputDirectory();
		LogLine(L"Saving to " + outputDir);
		WritePushGroupsToFile(groups, _userDir, outputDir);
	}

	void TakeUserDirectory()
	{
		bool proceed = false;
		char yesNoInput;
		do
		{
			LogLine("Input the repo directory for search: ");
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

	std::wstring TakeUserOutputDirectory()
	{
		bool proceed = false;
		char yesNoInput;
		std::wstring outputFilePathUser;
		do
		{
			LogLine("Output file directory: ");
			std::wcin >> outputFilePathUser;
			std::replace(outputFilePathUser.begin(), outputFilePathUser.end(), '/', '\\');
			LogLine(L"Your directory:  " + outputFilePathUser + L"   [y/n] ?");
			std::cin >> yesNoInput;
			if (yesNoInput == 'y')
				proceed = true;
			else
			{
				LogLine("Ok, start over");
				outputFilePathUser.clear();
				continue;
			}
		}
		while (!proceed);
		return outputFilePathUser;
	}

	export std::vector<FilesGroup*> MakePushGroups(std::wstring parentPath)
	{
		using namespace std;
		auto groups = std::vector<FilesGroup*>();
		groups.reserve(10);
		CreatePushGroupsRecursive(parentPath, groups, true);
		auto countT = to_string(groups.size());
		LogLine(string("Created ") + countT + string(" Groups"));
		int i = 0;
		uint32_t totalSize = 0;
		for (FilesGroup* groupPtr : groups)
		{
			if (groupPtr == nullptr)
				continue;
			i++;
			totalSize += groupPtr->totalSize;
			/*LogLine(string("\nPush ") + to_string(i));
			LogLine("=== === === ===");
			LogLine(string("Size ") + SizeToMbStrA(groupPtr->totalSize)
				+ " Folders count: " + to_string(groupPtr->localPaths.size()));
			for (auto p : groupPtr->localPaths)
			{
				LogLine(wstring(L"\t") + p);
			}*/
		}
		LogLine(string("\n\nTOTAL GROUPS SIZE ") + SizeToMbStrA(totalSize));
		return groups;
	}

	void CreatePushGroupsRecursive(std::wstring mainDirPath,
		std::vector<FilesGroup*>& outGroups, bool checkOwnSize)
	{
		using namespace std;
		if (checkOwnSize)
		{
			uint64_t dirSize = 0;
			GetDirectorySize(mainDirPath, dirSize);
			LogLine(wstring(L"Checked own size: ") + SizeToMbStr(dirSize));
			if (dirSize <= MAX_PUSH_SIZE)
			{
				auto groupPtr = new FilesGroup();
				FilesGroup& group = *groupPtr;
				group.localPaths.push_back(mainDirPath);
				group.totalSize = dirSize;
				outGroups.push_back(groupPtr);
				return;
			}
		}

		//LogLine(mainDirPath + std::wstring(L" Is Too Big. Step into to create new sub groups"));
		WIN32_FIND_DATA dataFind;
		HANDLE hFind;
		wstring parentPathAst = mainDirPath + std::wstring(L"\\*");
		hFind = FindFirstFile(parentPathAst.c_str(), &dataFind);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			LogError("INVALID_HANDLE_VALUE! Cannot continue");
			return;
		}
		if (dataFind.dwFileAttributes != CodeDirectory_Win)
		{
			LogError("Not a directory!");
			return;
		}
		FindNextFile(hFind, &dataFind); // .
		FindNextFile(hFind, &dataFind); // ..
		FilesGroup* groupPtr = new FilesGroup();
		outGroups.push_back(groupPtr);
		do
		{
			if (dataFind.dwFileAttributes != CodeDirectory_Win)
				continue;
			uint64_t subDirSize = 0;
			auto subDir = wstring(mainDirPath) + wstring(LR"(\)") + dataFind.cFileName;
			GetDirectorySize(subDir, subDirSize);
			//LogLine(wstring(L"Size: ") + SizeToMbStr(subDirSize) + wstring(L" ") + subDir);
			if (subDirSize > MAX_PUSH_SIZE)
			{
				//LogLine("MAX PUSH SIZE EXCEEDED");
				CreatePushGroupsRecursive(subDir, outGroups, false);
			}
			auto newSize = (*groupPtr).totalSize + subDirSize;
			if (newSize <= MAX_PUSH_SIZE)
			{
				(*groupPtr).totalSize = newSize;
				(*groupPtr).localPaths.push_back(subDir);
				//LogLine(subDir);
				//LogLine(string("Adding To Prev. Group, GroupSize: ") + SizeToMbStrA(newSize));
			}
			else
			{
				//LogLine(string("Creating new Files group. Size total: ") + SizeToMbStrA(newSize));
				groupPtr = new FilesGroup();
				(*groupPtr).totalSize = subDirSize;
				(*groupPtr).localPaths.push_back(subDir);
				outGroups.push_back(groupPtr);
			}
		}
		while (FindNextFile(hFind, &dataFind) != 0);
		FindClose(hFind);
		
		// Add self in the end, to include all non-included small files that are not in folders
		groupPtr = new FilesGroup();
		groupPtr->localPaths.push_back(mainDirPath);
		LogLine(L"ADDED SELF PARENT FOLDER:    " + mainDirPath);
		outGroups.push_back(groupPtr);
	}


	export void WritePushGroupsToFile(std::vector<FilesGroup*>& groups, std::wstring parentDir, std::wstring outputDir)
	{
		using namespace std;

		std::ofstream outputFile;
		outputFile.open(outputDir);
		if (!outputFile.is_open())
		{
			LogError("Failed to open or create the output file");
			return;
		}
		
		int i = 0;
		int cutPos = parentDir.length() + 1;
		for (auto group : groups)
		{
			if (group->localPaths.size() == 0)
			{
				LogLine("No paths saved, skip");
				continue;
			}
			i++;
			string pushStr = string("Push ") + to_string(i);
			LogLine(pushStr);
			outputFile << pushStr << "\n";
			auto commandLineStr = std::string("git add ");
			for (auto wPath : group->localPaths)
			{
				string path = std::string(wPath.begin(), wPath.end());
				string localPath = path.substr(cutPos);
				commandLineStr += string("\"") + localPath + string("\"") + string(" ");
			}
			LogLine(commandLineStr);
			outputFile << commandLineStr << "\n";
		}
		outputFile.close();

	}
}