#include "producer.h"
#include <core/logger.h>
#include <idl/fastq_generated.h>
#include <idl/system_asserts.h> // do not remove, ensures important assertions
#include <flatbuffers/flatbuffers.h>

namespace FastQ {

static const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};

void Producer::Produce()
{
	LOG(INFO, LM_PRODUCER, "producing")
	flatbuffers::FlatBufferBuilder builder(1024);
}

}