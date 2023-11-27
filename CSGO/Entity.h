#pragma once
#include "pch.h"

/* auto padding */
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}
#define DEFINE_MEMBER_0(type, name) type name

struct Vector3 {
	float x, y, z;
};

class Entity
{
public:
	union {
		DEFINE_MEMBER_0(uintptr_t, vTable);
		DEFINE_MEMBER_N(Vector3, feetPos, 0x2C);
		DEFINE_MEMBER_N(Vector3, Velocity, 0x17C);
		DEFINE_MEMBER_N(Vector3, Position, 0x1DC);//headPos
		DEFINE_MEMBER_N(int32_t, health, 0x230);
		DEFINE_MEMBER_N(Vector3, GraphicalPos, 0x2DC);
		DEFINE_MEMBER_N(BYTE, team, 0x314);
		DEFINE_MEMBER_N(Vector3, viewAngles, 0x434);
	};
}; //Size=0x0840

struct EntityList {
	std::vector<Entity*> entities;
};