#include "logger.h"

namespace FastQ {

static uint8_t FASTQ_GLOBAL_LOG_LEVEL = FASTQ_INFO;

void SetGlobalLogLevel(int logLevel)
{
	FASTQ_GLOBAL_LOG_LEVEL = logLevel;
}

uint8_t GetGlobalLogLevel()
{
	return FASTQ_GLOBAL_LOG_LEVEL;
}

}