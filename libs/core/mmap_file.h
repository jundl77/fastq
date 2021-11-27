#pragma once

#include <string>

namespace FastQ {

enum MmapProtMode
{
	NO_SET = 1,
	READ_WRITE,
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
	void Close();
	const std::string& GetName() const { return mShmFilename; }
	void* GetAddress() const;

private:
	int ShmOpenFile(MmapProtMode);
	void CloseFile();
	int MmapProtModeToProt(MmapProtMode);
	int MmapProtModeToOMode(MmapProtMode);

private:
	std::string mShmFilename;
	int mFd {0};
	void* mAddr;
	int mMappedSize {0};
	bool mIsOpen {false};
	bool mIsMapped {false};
	MmapProtMode mProtMode {MmapProtMode::NO_SET};
};

}