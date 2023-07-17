#pragma once
#include <vector>
#include <fstream>
#include <cstdint>
#include "BbsPmo.h"
#include "Tim2.h"

struct PmpHeader
{
    uint32_t magic;
	uint32_t unk_04;
	uint32_t unk_08;
	uint32_t unk_0C;
	uint16_t obj_count;
	uint16_t unk_12;
	uint32_t unk_14;
	uint16_t unk_18;
	uint16_t tex_count;
	uint32_t tex_index_offset;
};

struct PmpObjEntry
{
    float loc[3];
	float rot[3];
	float scale[3];
	uint32_t offset;
	uint32_t unk_28;	// Seems to always be 0
	uint16_t flags;
	uint16_t unk_2E; 	// Probably an object id number

    PmoFile data;
};

struct PmpTexEntry
{
	uint32_t offset;
	char name[0xC];
	uint32_t unk_10[4];

    Tm2File data;
};

class PmpFile
{
public:
    PmpHeader header;
    std::vector<PmpObjEntry> objects;
    std::vector<PmpTexEntry> textures;

	uint16_t objectCount();
	uint16_t textureCount();

    static PmpFile ReadPmpFile(std::ifstream& file, std::streamoff base = 0);
};