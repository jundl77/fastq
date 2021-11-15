#pragma once

#include <idl/fastq.h>
#include "logger.h"

namespace FastQ {

class FastQCore {
public:
	virtual ~FastQCore() = default;

	const Idl::Header& GetHeader();

protected:
	explicit FastQCore(const LogModule& module);

	void Init(Idl::FastQueue* fastQueue);
	void LogFastQHeader() const;
	int GetTotalFastQSize() const;

	inline uint64_t NextFramePosition(uint32_t lastWritePosition, int sizeToWrite) const
	{
		uint64_t nextPosition = 0;
		if (lastWritePosition + sizeToWrite < mPayloadSize)
			nextPosition = lastWritePosition + sizeToWrite;
		return nextPosition;
	}

protected:
	const LogModule& mLogModule;
	Idl::FastQueue* mFastQueue;
	uint64_t mPayloadSize {0};
};

}