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

class IFastQHandler
{
public:
	virtual ~IFastQHandler() = default;

	virtual void OnConnected() = 0;
	virtual void OnDisconnected(const std::string& reason) = 0;
	virtual void OnData(u_int32_t type, void* data, u_int32_t size) = 0;
};

class Consumer : public FastQCore
{
public:
	Consumer(std::string shmFilename, IFastQHandler&);

	void Start();
	void Shutdown(); // ensures clean closing of shm
	bool Poll();

	bool IsConnected() const { return mConnected; }

private:
	uint32_t ReadData(uint32_t lastReadPosition, void* data, uint32_t size);
	void AssertInSync();
	void ValidateHeader();

private:
	std::string mShmFilename;
	IFastQHandler& mHandler;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	Idl::FastQueue* mFastQueue;
	std::vector<uint8_t> mCurrentReadBuffer;
	uint32_t mFileSize {0};
	uint32_t mPayloadSize {0};
	uint32_t mWrapAroundCounter {0};
	uint32_t mLastReadPosition {0};
	bool mConnected {false};

	friend class FastQTestFixture;
};

}