#pragma once

#include <cstddef>
#include <cstdint>

#define FASTQ_PROTOCOL_NAME "fastq_protocol"

namespace FastQ::Idl {

static constexpr size_t FASTQ_HEADER_SIZE = 44;
static constexpr size_t FASTQ_SIZE_WITHOUT_PAYLOAD = FASTQ_HEADER_SIZE + 12;
static constexpr uint32_t FASTQ_MAJOR_VERSION = 1;
static constexpr uint32_t FASTQ_MINOR_VERSION = 1;

#pragma pack(push, 1)

template<uint32_t FrameSizeT>
struct Header
{
	explicit Header(uint32_t magicNumber, uint32_t maxFrameCount)
		: mMagicNumber(magicNumber)
		, mMaxFrameCount(maxFrameCount)
		, mPayloadSize(mFrameSize * mMaxFrameCount)
	{}

	const char mProtocolName[16] = FASTQ_PROTOCOL_NAME;
	const uint32_t mVersionMajor {FASTQ_MAJOR_VERSION};
	const uint32_t mVersionMinor {FASTQ_MINOR_VERSION};
	const uint32_t mMagicNumber {0};
	const uint32_t mFrameSize {FrameSizeT};
	const uint32_t mMaxFrameCount {0};
	const uint64_t mPayloadSize {0};
};
static_assert(sizeof(Header<1>) == FASTQ_HEADER_SIZE);

template<uint32_t FrameSizeT>
struct FastQueue
{
	explicit FastQueue(uint32_t magicNumber, uint32_t maxFrameCount)
		: mHeader(magicNumber, maxFrameCount)
	{}

	const Header<FrameSizeT> mHeader;
	uint32_t mHeartbeatCount {0};
	int32_t mWrapAroundCount {-1};
	int32_t mLastWriteIndex {-1}; // -1 means we have not written yet, the queue is empty
	uint8_t* mPayload;
};
static_assert(sizeof(FastQueue<1>) == FASTQ_SIZE_WITHOUT_PAYLOAD + sizeof(uint8_t*));

#pragma pack(pop)

}