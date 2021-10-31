#pragma once

#include <string>

namespace FastQ {

enum MmapProtMode
{
	READ_WRITE = 1,
	READ_ONLY
};

class MmappedFile
{
public:
	explicit MmappedFile(std::string shmFilename);
	~MmappedFile();

	void Create(int initialSize);
	void Mmap(int size, MmapProtMode);
	void Munmap();
	const std::string& GetName() const { return mShmFilename; }
	void* GetAddress() const;

private:
	int OpenFile(MmapProtMode);
	int MmapProtModeToProt(MmapProtMode);
	int MmapProtModeToOMode(MmapProtMode);

	std::string mShmFilename;
	int mFd;
	void* mAddr;
	int mMappedSize;
	bool mIsMapped {false};
};

}