#pragma once

#include <string>

namespace FastQ {

class Consumer
{
public:
	Consumer(std::string shmFilename, int filesizeMb);
	void Start();

private:
	int LoadMmappedFile(const std::string& shmFilename, int filesizeMb);

	std::string mShmFilename;
	int mFilesizeMb;
	int mFd {0};
};

}