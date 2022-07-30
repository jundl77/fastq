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
	mPayload = mFastQueue->mData;
	LogFastQHeader();
}

void FastQCore::LogFastQHeader() const
{
	LOG(FASTQ_INFO, mLogModule, "fastq header: ");
	LOG(FASTQ_INFO, mLogModule, "  protocol name:   %s", mFastQueue->mHeader.mProtocolName)
	LOG(FASTQ_INFO, mLogModule, "  major version:   %d", mFastQueue->mHeader.mVersionMajor)
	LOG(FASTQ_INFO, mLogModule, "  minor version:   %d", mFastQueue->mHeader.mVersionMinor)
	LOG(FASTQ_INFO, mLogModule, "  magic number:    %d", mFastQueue->mHeader.mMagicNumber)
	LOG(FASTQ_INFO, mLogModule, "  file size:       %d", mFastQueue->mHeader.mFileSize)
	LOG(FASTQ_INFO, mLogModule, "  payload size:    %d", mFastQueue->mHeader.mPayloadSize)
}

}