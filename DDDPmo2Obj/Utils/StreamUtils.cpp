#include "StreamUtils.h"

std::vector<uint8_t> ReadBlob(std::istream& stream, size_t size)
{
	if (!stream.good())
	{
		if (stream.eof())
			throw new std::runtime_error("Unexpected EOF");
		else
			throw new std::runtime_error("Unknown IO Error");
	}
	std::vector<uint8_t> data = std::vector<uint8_t>(size);
	for (size_t i = 0; i < size; i++)
	{
		//data.push_back(stream.get());
		data[i] = stream.get();
		if (!stream.good())
		{
			if (stream.eof())
				throw new std::runtime_error("Unexpected EOF");
			else
				throw new std::runtime_error("Unknown IO Error");
		}
	}
	return data;
}

void Realign(std::istream& stream, size_t size)
{
	std::streamoff pos = stream.tellg();
	if (pos % size != 0)
	{
		stream.seekg((size - (pos % size)), std::ios_base::cur);
	}
}

std::string ReadCString(std::istream& stream)
{
	if (!stream.good()) return std::string();
	std::string str = std::string();
	char c = stream.get();
	while (c != '\0' && stream.good())
	{
		str += c;
		c = stream.get();
	}
	return str;
}