#pragma once
#include <iostream>
#include <vector>
#include <cstdint>

std::vector<uint8_t> ReadBlob(std::istream& stream, size_t size);

void Realign(std::istream& stream, size_t alignment);

template<typename T1,
	typename std::enable_if<std::is_trivially_copyable_v<T1>
	&& std::is_standard_layout_v<T1>>::type* = nullptr>
T1 ReadStream(std::istream& stream)
{
	T1 var;
	stream.read((char*)(&var), sizeof(T1));
	return var;
}

template<typename T1,
	typename std::enable_if<std::is_trivially_copyable_v<T1>
	&& std::is_standard_layout_v<T1>>::type* = nullptr>
void ReadStreamArr(std::istream& stream, T1* arr, size_t count)
{
	for (int i = 0; i < count; i++) arr[i] = ReadStream<T1>(stream);
}

std::string ReadCString(std::istream& stream);