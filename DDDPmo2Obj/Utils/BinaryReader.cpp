#include "BinaryReader.h"

BinaryReader::BinaryReader(std::string filepath) : cachedSize((uint64)-1), fileStream(filepath, std::ios::binary)
{
	
}

BinaryReader::~BinaryReader()
{
	if (fileStream.is_open()) fileStream.close();
}

bool BinaryReader::Ok()
{
	return fileStream.good();
}

bool BinaryReader::Eof()
{
	return fileStream.eof();
}

bool BinaryReader::IsOpen()
{
	return fileStream.is_open();
}

uint64 BinaryReader::GetFileSize()
{
	if (cachedSize == (uint64)-1) CalcFileSize();

	return cachedSize;
}

void BinaryReader::Skip(int64 offset)
{
	fileStream.seekg((std::streamoff)offset, std::ios_base::cur);
}

void BinaryReader::Seek(uint64 position)
{
	fileStream.seekg((std::streamoff)position, std::ios_base::beg);
}

void BinaryReader::CalcFileSize()
{
	if (!Ok()) return;
	
	auto prevPos = fileStream.tellg();
	fileStream.seekg(0, std::ios_base::end);
	auto endPos = fileStream.tellg();
	fileStream.seekg(prevPos, std::ios_base::beg);
	cachedSize = endPos;
}

template <typename T>
inline T read_impl(std::ifstream& f)
{
	T val;
	f.read((char*)&val, sizeof(T));
	return val;
}

double BinaryReader::ReadDouble()
{
	return read_impl<double>(fileStream);
}

float BinaryReader::ReadFloat()
{
	return read_impl<float>(fileStream);
}

int64 BinaryReader::ReadI64()
{
	return read_impl<int64>(fileStream);
}

uint64 BinaryReader::ReadU64()
{
	return read_impl<uint64>(fileStream);
}

int32 BinaryReader::ReadI32()
{
	return read_impl<int32>(fileStream);
}

uint32 BinaryReader::ReadU32()
{
	return read_impl<uint32>(fileStream);
}

int16 BinaryReader::ReadI16()
{
	return read_impl<int16>(fileStream);
}

uint16 BinaryReader::ReadU16()
{
	return read_impl<uint16>(fileStream);
}

int8 BinaryReader::ReadI8()
{
	return read_impl<int8>(fileStream);
}

uint8 BinaryReader::ReadU8()
{
	return read_impl<uint8>(fileStream);
}

void BinaryReader::RawRead(char* pData, uint64 size)
{
	fileStream.read(pData, size);
}

BinaryReader& operator>>(BinaryReader& reader, double& d)
{
	d = reader.ReadDouble();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, float& f)
{
	f = reader.ReadFloat();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, uint64& u64)
{
	u64 = reader.ReadU64();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, int64& i64)
{
	i64 = reader.ReadI64();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, int32& i32)
{
	i32 = reader.ReadI32();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, uint32& u32)
{
	u32 = reader.ReadU32();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, int16& i16)
{
	i16 = reader.ReadI16();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, uint16& u16)
{
	u16 = reader.ReadU16();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, int8& i8)
{
	i8 = reader.ReadI8();
	return reader;
}

BinaryReader& operator>>(BinaryReader& reader, uint8& u8)
{
	u8 = reader.ReadU8();
	return reader;
}
