#include "producer.h"
#include <idl/fastq_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <fastq_generated.h>

namespace FastQ {

void Produce()
{
	flatbuffers::FlatBufferBuilder builder(1024);
	auto sword = CreateWeapon(builder, "w1", "w2");
}

}