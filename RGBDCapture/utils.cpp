#include "utils.h"

bool isExistingDir(string path)
{
	struct stat info;

	int dirRet = stat(path.c_str(), &info);
	return (info.st_mode & S_IFDIR);
}