#pragma once

#include <string>

namespace FastQ {

class Producer
{
public:
	Producer(std::string shmFilename, int filesizeMb);
	void Start();

private:
	int CreateMmappedFile(const std::string& shmFilename, int filesizeMb);

	std::string mShmFilename;
	int mFilesizeMb;
	int mFd {0};
};

}