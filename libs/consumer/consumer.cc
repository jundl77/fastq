#include "consumer.h"
#include <flatbuffers/flatbuffers.h>
#include <idl/fastq_generated.h>
#include <idl/system_asserts.h> // do not remove, ensures important assertions

static_assert(FLATBUFFERS_LITTLEENDIAN);

namespace FastQ {

void Consume()
{
	flatbuffers::FlatBufferBuilder builder(1024);
}

}