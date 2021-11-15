#include "fastq_core.h"

namespace FastQ {

const Idl::Header& FastQCore::GetHeader()
{
	return mFastQueue->mHeader;
}

FastQCore::FastQCore(const LogModule& module)
	: mLogModule(module)
{
}

void FastQCore::Init(Idl::FastQueue* fastQueue)
{
	mFastQueue = fastQueue;
	mPayloadSize = mFastQueue->mHeader.mPayloadSize;
	mFastQueue->mPayload = reinterpret_cast<uint8_t*>(mFastQueue) + Idl::FASTQ_SIZE_WITHOUT_PAYLOAD;
	LogFastQHeader();
}

void FastQCore::LogFastQHeader() const
{
	LOG(INFO, mLogModule, "fastq header: ");
	LOG(INFO, mLogModule, "  protocol name:   %s", mFastQueue->mHeader.mProtocolName)
	LOG(INFO, mLogModule, "  major version:   %d", mFastQueue->mHeader.mVersionMajor)
	LOG(INFO, mLogModule, "  minor version:   %d", mFastQueue->mHeader.mVersionMinor)
	LOG(INFO, mLogModule, "  magic number:    %d", mFastQueue->mHeader.mMagicNumber)
	LOG(INFO, mLogModule, "  payload size:    %llu", mFastQueue->mHeader.mPayloadSize)
}

int FastQCore::GetTotalFastQSize() const
{
	return Idl::FASTQ_SIZE_WITHOUT_PAYLOAD + mPayloadSize;
}

}