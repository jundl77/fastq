#pragma once

#include "logger.h"
#include <idl/fastq.h>

namespace FastQ {

template<uint32_t SizeT, uint32_t CountT>
void LogFastQHeader(const LogModule& logModule, const Idl::FastQueue<SizeT, CountT>* fastq)
{
	LOG(INFO, logModule, "fastq header: ")
	LOG(INFO, logModule, "  protocol name:   " << fastq->mHeader.mProtocolName)
	LOG(INFO, logModule, "  major version:   " << fastq->mHeader.mVersionMajor)
	LOG(INFO, logModule, "  minor version:   " << fastq->mHeader.mVersionMinor)
	LOG(INFO, logModule, "  magic number:    " << fastq->mHeader.mMagicNumber)
	LOG(INFO, logModule, "  heartbeat count: " << fastq->mHeader.mHeartbeatCount)
	LOG(INFO, logModule, "  item size:       " << fastq->mHeader.mItemSize)
	LOG(INFO, logModule, "  max item count:  " << fastq->mHeader.mMaxItemCount)
	LOG(INFO, logModule, "  payload size:    " << fastq->mHeader.mPayloadSize)
}

template<uint32_t SizeT, uint32_t CountT>
int GetTotalFastQSize()
{
	return Idl::FASTQ_SIZE_WITHOUT_PAYLOAD + SizeT * CountT;
}

}