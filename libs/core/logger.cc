#include "logger.h"

namespace FastQ {

static uint8_t GLOBAL_LOG_LEVEL = INFO;

void SetGlobalLogLevel(int logLevel)
{
	GLOBAL_LOG_LEVEL = logLevel;
}

uint8_t GetGlobalLogLevel()
{
	return GLOBAL_LOG_LEVEL;
}

}