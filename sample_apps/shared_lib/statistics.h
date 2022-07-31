#pragma once

#include <core/logger.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace FastQ::SampleApps {

template<typename V>
class StatisticsCollector
{
public:
	StatisticsCollector(V initialMinValue, V initialMaxValue)
		: mMin(initialMinValue)
		, mMax(initialMaxValue)
	{
	}

	inline void Record(V value)
	{
		mSum += value;
		mNumElementsSeen += 1;

		if (value > mMax)
		{
			mMax = value;
		}
		if (value < mMin)
		{
			mMin = value;
		}
	}

	inline void Report(const LogModule& logModule, const std::string& tagName, const std::string& statName)
	{
		LOG(FASTQ_INFO, logModule, "[%s] {\"%s.count\": %llu}", tagName.c_str(), statName.c_str(), mNumElementsSeen);
		LOG(FASTQ_INFO, logModule, "[%s] {\"%s.mean\":  %f}", tagName.c_str(), statName.c_str(), GetMean());
		LOG(FASTQ_INFO, logModule, "[%s] {\"%s.min\":   %f}", tagName.c_str(), statName.c_str(), static_cast<double>(mMin));
		LOG(FASTQ_INFO, logModule, "[%s] {\"%s.max\":   %f}", tagName.c_str(), statName.c_str(), static_cast<double>(mMax));
	}

	inline uint64_t GetCount() const { return mNumElementsSeen; }
	inline double GetMean() const
	{
		if (mNumElementsSeen == 0)
			return 0;
		return mSum / mNumElementsSeen;
	}

private:
	uint64_t mNumElementsSeen {0};
	V mSum {0};

	V mMin {0};
	V mMax {0};
};

}