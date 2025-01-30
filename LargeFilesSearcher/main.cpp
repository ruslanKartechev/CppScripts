#define WIN_PLATFORM
import LF_searcher;
import push_helper;
import Logger;
import dir_utils;
import <iostream>;
import <string>;

using namespace SleepDev;

void GetPushGroupsForTestRepo()
{
    std::wstring dirPath = LR"(C:\MyUnityWork\Projects\mh_it_2)";
    std::wstring assetsPath = dirPath + std::wstring(LR"(\Assets)");
    std::wstring outputPath = LR"(C:\Users\karte\OneDrive\Desktop\repo_ouptut.txt)";

    auto groups = MakePushGroups(assetsPath);
    WritePushGroupsToFile(groups, dirPath, outputPath);
}

int main()
{
    GetPushGroupsForTestRepo();
    return -1;

    using namespace std;
    const int MODE_SEARCH = 1;
    const int MODE_GROUPS = 2;

    LogLine("Hi! What to do?\n[1] - Search Large Files\n[2] - Group Files for commits under github push size limit");
    int mode = -1;
    do
    {
        int input;
        std::cin >> input;
        switch (input)
        {
        case 1:
            mode = MODE_SEARCH;
            break;
        case 2:
            mode = MODE_GROUPS;
            break;
        default:
            mode = -1;
            LogLine("Cannot process this input (( Try again");
            break;
        }
    }
    while (mode < 0);
    switch (mode)
    {
    case MODE_SEARCH:
        BeginLFSearch();
        break;
    case MODE_GROUPS:
        BeginPushHelper();
        break;
    }

}
