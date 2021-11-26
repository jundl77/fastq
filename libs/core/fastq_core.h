#pragma once

#include <idl/fastq.h>
#include "logger.h"

namespace FastQ {

class FastQCore {
public:
	virtual ~FastQCore() = default;

	const Idl::Header& GetHeader();

public:
	static inline uint64_t CreateLastWriteInfo(uint32_t lastWritePosition, uint32_t wrapAroundCount)
	{
		// wrapAroundCount only uses 28 bytes
		uint64_t lastWriteInfo = lastWritePosition;
		lastWriteInfo = lastWriteInfo << 28;
		return lastWriteInfo | wrapAroundCount;
	}

	static inline uint32_t GetLastWritePosition(uint64_t lastWriteInfo)
	{
		return (uint32_t) (lastWriteInfo >> 28);
	}

	static inline uint32_t GetWrapAroundCount(uint64_t wrapAroundCount)
	{
		return (uint32_t) (wrapAroundCount & 0xfffffff);
	}

protected:
	explicit FastQCore(const LogModule& module);

	void Init(Idl::FastQueue* fastQueue);
	void LogFastQHeader() const;
	uint8_t* GetPayloadPointer() const { return mPayload; };

	inline uint32_t NextFramePosition(uint32_t lastPosition, int size) const
	{
		uint32_t nextPosition = lastPosition + size;
		if (nextPosition > mPayloadSize)
			nextPosition = 0;
		return nextPosition;
	}

private:
	const LogModule& mLogModule;
	Idl::FastQueue* mFastQueue;
	uint32_t mPayloadSize {0};
	uint8_t* mPayload {nullptr};
};

}