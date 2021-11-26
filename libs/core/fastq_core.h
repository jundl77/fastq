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
	uint8_t* GetPayloadPointer() const { return mPayload; };

	inline uint32_t NextFramePosition(uint32_t lastPosition, int size) const
	{
		uint32_t nextPosition = 0;
		if (lastPosition + size <= mPayloadSize)
			nextPosition = lastPosition + size;
		return nextPosition;
	}

private:
	const LogModule& mLogModule;
	Idl::FastQueue* mFastQueue;
	uint32_t mPayloadSize {0};
	uint8_t* mPayload {nullptr};
};

}