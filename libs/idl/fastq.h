#pragma once

#include <cstddef>
#include <cstdint>

#define FASTQ_PROTOCOL_NAME "fastq_protocol"

namespace FastQ::Idl {

static constexpr size_t FASTQ_FRAMING_HEADER_SIZE = 8;
static constexpr size_t FASTQ_HEADER_SIZE = 36;
static constexpr size_t FASTQ_SIZE_WITHOUT_PAYLOAD = FASTQ_HEADER_SIZE + 12;
static constexpr uint32_t FASTQ_MAJOR_VERSION = 1;
static constexpr uint32_t FASTQ_MINOR_VERSION = 1;

#pragma pack(push, 1)

struct FramingHeader
{
	FramingHeader() = default;
	FramingHeader(uint32_t type, uint32_t size)
	: mType(type)
	, mSize(size)
	{}

	uint32_t mType {0};
	uint32_t mSize {0};
};
static_assert(sizeof(FramingHeader) == FASTQ_FRAMING_HEADER_SIZE);

struct Header
{
	Header(uint32_t magicNumber, uint32_t fileSize)
		: mMagicNumber(magicNumber)
		, mFileSize(fileSize)
		, mPayloadSize(mFileSize - Idl::FASTQ_SIZE_WITHOUT_PAYLOAD)
	{}

	const char mProtocolName[16] = FASTQ_PROTOCOL_NAME;
	const uint32_t mVersionMajor {FASTQ_MAJOR_VERSION};
	const uint32_t mVersionMinor {FASTQ_MINOR_VERSION};
	const uint32_t mMagicNumber {0};
	const uint32_t mFileSize {0};
	const uint32_t mPayloadSize {0};
};
static_assert(sizeof(Header) == FASTQ_HEADER_SIZE);

struct FastQueue
{
	FastQueue(uint32_t magicNumber, uint32_t fileSize)
		: mHeader(magicNumber, fileSize)
	{}

	const Header mHeader;
	uint32_t mHeartbeatCount {0};
	uint32_t mWrapAroundCount {0};
	uint32_t mLastWritePosition {0};
	// data is here
};
static_assert(sizeof(FastQueue) == FASTQ_SIZE_WITHOUT_PAYLOAD);

#pragma pack(pop)

}