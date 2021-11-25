#pragma once

#include <core/mmap_file.h>
#include <core/fastq_core.h>
#include <core/throw_if.h>
#include <core/logger.h>
#include <idl/fastq.h>
#include <memory>
#include <cstring>
#include <vector>

namespace FastQ {

class FastQHandler
{
public:
	virtual ~FastQHandler() = default;
	virtual void OnData(u_int32_t type, void* data, u_int32_t size) = 0;
};

class Consumer : public FastQCore
{
public:
	Consumer(std::string shmFilename, FastQHandler&);

	void Start();
	bool Poll();

private:
	uint32_t ReadData(uint32_t lastReadPosition, void* data, uint32_t size);
	bool AssertInSync();
	void ValidateHeader();

private:
	std::string mShmFilename;
	FastQHandler& mHandler;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	Idl::FastQueue* mFastQueue;
	std::vector<uint8_t> mCurrentReadBuffer;
	uint32_t mFileSize {0};
	uint32_t mPayloadSize {0};
	int32_t mWrapAroundCount {0};
	int32_t mLastReadPosition {0};
};

}