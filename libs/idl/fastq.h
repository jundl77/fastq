#pragma once

#include <cstddef>
#include <cstdint>

#define FASTQ_PROTOCOL_NAME "fastq_protocol"

namespace FastQ::Idl {

static constexpr size_t FASTQ_HEADER_SIZE = 36;
static constexpr size_t FASTQ_SIZE_WITHOUT_PAYLOAD = FASTQ_HEADER_SIZE + 20;
static constexpr uint32_t FASTQ_MAJOR_VERSION = 1;
static constexpr uint32_t FASTQ_MINOR_VERSION = 1;

#pragma pack(push, 1)

struct Header
{
	explicit Header(uint32_t magicNumber, uint32_t payloadSize)
		: mMagicNumber(magicNumber)
		, mPayloadSize(payloadSize)
	{}

	const char mProtocolName[16] = FASTQ_PROTOCOL_NAME;
	const uint32_t mVersionMajor {FASTQ_MAJOR_VERSION};
	const uint32_t mVersionMinor {FASTQ_MINOR_VERSION};
	const uint32_t mMagicNumber {0};
	const uint64_t mPayloadSize {0};
};
static_assert(sizeof(Header) == FASTQ_HEADER_SIZE);

struct FastQueue
{
	explicit FastQueue(uint32_t magicNumber, uint64_t payloadSize)
		: mHeader(magicNumber, payloadSize)
	{}

	const Header mHeader;
	uint32_t mHeartbeatCount {0};
	int32_t mWrapAroundCount {0};
	int32_t mLastWritePosition {0};
	uint8_t* mPayload {nullptr};
};
static_assert(sizeof(FastQueue) == FASTQ_SIZE_WITHOUT_PAYLOAD);

#pragma pack(pop)

}