export module Logger;
import <iostream>;
import <string>;
import <algorithm>;

namespace SleepDev
{
	export void LogLine(const char* str)
	{
		std::cout << str << std::endl;
	}

	export void LogError(const char* str)
	{
		std::cout << "ERROR! " << str << std::endl;
	}

	export void LogLine(const std::string& str)
	{
		std::cout << str << std::endl;
	}

	export void LogLine(const std::wstring& str)
	{
		std::wcout << str << std::endl;
	}

	export void LogEmptyLine(size_t count = 1)
	{
		for (auto i = 0; i < count; i++)
			std::cout << std::endl;
	}

	export void LogAndSpace(std::string str, char space = ' ')
	{
		std::cout << str << space;
	}


	export void LogAndSpace(std::string str, std::string space = " ")
	{
		std::cout << str << space;
	}

	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}

	export void ParseString(std::string& str)
	{
		//auto token = strtok(str, "0");

	}

	export void ReplaceSubstrings(std::string& str, const std::string& from, const std::string& to)
	{
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

}