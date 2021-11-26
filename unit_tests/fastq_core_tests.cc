#include <core/fastq_core.h>
#include <gtest/gtest.h>

namespace FastQ::Testing {

TEST(FastQCoreTests, LastWriteInfo_ReadWrite)
{
	uint32_t lastWritePosition = 54543;
	uint32_t wrapAroundCount = 544;
	uint64_t lastWriteInfo = FastQCore::CreateLastWriteInfo(lastWritePosition, wrapAroundCount);

	EXPECT_EQ(lastWritePosition, FastQCore::GetLastWritePosition(lastWriteInfo));
	EXPECT_EQ(wrapAroundCount, FastQCore::GetWrapAroundCount(lastWriteInfo));
}

TEST(FastQCoreTests, LastWriteInfo_ReadWrite_ZeroAndOne)
{
	uint32_t lastWritePosition = 0;
	uint32_t wrapAroundCount = 1;
	uint64_t lastWriteInfo = FastQCore::CreateLastWriteInfo(lastWritePosition, wrapAroundCount);

	EXPECT_EQ(lastWritePosition, FastQCore::GetLastWritePosition(lastWriteInfo));
	EXPECT_EQ(wrapAroundCount, FastQCore::GetWrapAroundCount(lastWriteInfo));
}

TEST(FastQCoreTests, LastWriteInfo_ReadWrite_MaxValue)
{
	uint32_t lastWritePosition = std::numeric_limits<uint32_t>::max();
	uint32_t wrapAroundCount = 268435455; // 268435456 is max for 2^28
	uint64_t lastWriteInfo = FastQCore::CreateLastWriteInfo(lastWritePosition, wrapAroundCount);

	EXPECT_EQ(lastWritePosition, FastQCore::GetLastWritePosition(lastWriteInfo));
	EXPECT_EQ(wrapAroundCount, FastQCore::GetWrapAroundCount(lastWriteInfo));
}

}