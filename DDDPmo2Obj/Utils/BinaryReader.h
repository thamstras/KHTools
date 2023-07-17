#pragma once
#include <fstream>
#include <string>
#include "BinaryTypes.h"

/// <summary>
/// Wrapper around an fstream for reading binary data easy.
/// NOTE: Assumes file and platform are same endianness.
/// </summary>
class BinaryReader
{
public:
	BinaryReader(std::string filepath);
	~BinaryReader();

	BinaryReader(BinaryReader& other) = delete;
	BinaryReader& operator=(BinaryReader& other) = delete;

	// TODO: Move constructor?
	// TODO: Move operator?

	bool Ok();
	bool Eof();
	bool IsOpen();

	uint64 GetFileSize();

	void Skip(int64 offset);
	void Seek(uint64 position);
	uint64 Tell();

	double ReadDouble();
	float ReadFloat();
	int64 ReadI64();
	uint64 ReadU64();
	int32 ReadI32();
	uint32 ReadU32();
	int16 ReadI16();
	uint16 ReadU16();
	int8 ReadI8();
	uint8 ReadU8();

	void RawRead(char* pData, uint64 size);

	friend BinaryReader& operator>>(BinaryReader& reader, double& d);
	friend BinaryReader& operator>>(BinaryReader& reader, float& f);
	friend BinaryReader& operator>>(BinaryReader& reader, int64& i64);
	friend BinaryReader& operator>>(BinaryReader& reader, uint64& u64);
	friend BinaryReader& operator>>(BinaryReader& reader, int32& i32);
	friend BinaryReader& operator>>(BinaryReader& reader, uint32& u32);
	friend BinaryReader& operator>>(BinaryReader& reader, int16& i16);
	friend BinaryReader& operator>>(BinaryReader& reader, uint16& u16);
	friend BinaryReader& operator>>(BinaryReader& reader, int8& i8);
	friend BinaryReader& operator>>(BinaryReader& reader, uint8& u8);

private:
	std::ifstream fileStream;
	uint64 cachedSize;

	void CalcFileSize();
};