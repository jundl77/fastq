#include "producer.h"
#include <flatbuffers/flatbuffers.h>
#include <fastq_generated.h>

namespace FastQ {

void Produce()
{
	flatbuffers::FlatBufferBuilder builder(1024);
	auto weapon_one_name = builder.CreateString("Sword");
	short weapon_one_damage = 3;

	auto weapon_two_name = builder.CreateString("Axe");
	short weapon_two_damage = 5;

	auto sword = CreateWeapon(builder, weapon_one_name, weapon_one_damage);
	auto axe = CreateWeapon(builder, weapon_two_name, weapon_two_damage);
}

}