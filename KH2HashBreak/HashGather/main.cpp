#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

struct IDX_RECORD
{
	uint32 hash32;
	uint16 hash16;
	struct
	{
		uint16 blockCount : 14;
		uint16 isCompressed : 1;
		uint16 isStreamed : 1;
	} flags;
	int32 offset;
	int32 uncompressedSize;
};

uint8* g_fileBuf;

bool LoadIDX(const char* path)
{
	FILE* fp;
	char* fileBuf;
	
	fp = fopen(path, "rb");
	if (fp == nullptr)
	{
		std::cerr << "Error opening IDX!" << std::endl;
		return false;
	}

	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	rewind(fp);

	fileBuf = (char*)malloc(fsize);
	if (fileBuf == nullptr)
	{
		std::cerr << "Failed to allocate memory for file buffer!" << std::endl;
		fclose(fp);
		return false;
	}

	size_t result = fread(fileBuf, 1, fsize, fp);
	if (result != fsize)
	{
		std::cerr << "Read error" << std::endl;
		free(fileBuf);
		fclose(fp);
		return false;
	}

	g_fileBuf = (uint8*)fileBuf;
	return true;
}

void ReadIDX()
{
	uint32 fileCount;

	fileCount = *((uint32*)g_fileBuf);
	IDX_RECORD* fileList = (IDX_RECORD*)g_fileBuf + 4;
	for (int r = 0; r < fileCount; r++)
	{

	}
}