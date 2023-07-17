#include "BbsPmp.h"
#include <stdexcept>
#include "..\Utils\MagicCode.h"
#include "..\Utils\StreamUtils.h"

constexpr uint32_t pmpMagic = MagicCode('P', 'M', 'P', '\0');

PmpHeader ParsePmpHeader(std::ifstream& file)
{
    PmpHeader header;
    header.magic = ReadStream<uint32_t>(file);
    if (header.magic != pmpMagic)
    {
        throw std::runtime_error("File is not a valid PMP file! (magic code fail)");
    }
    header.unk_04 = ReadStream<uint32_t>(file);
    header.unk_08 = ReadStream<uint32_t>(file);
    header.unk_0C = ReadStream<uint32_t>(file);
    header.obj_count = ReadStream<uint16_t>(file);
    header.unk_12 = ReadStream<uint16_t>(file);
    header.unk_14 = ReadStream<uint32_t>(file);
    header.unk_18 = ReadStream<uint16_t>(file);
    header.tex_count = ReadStream<uint16_t>(file);
    header.tex_index_offset = ReadStream<uint32_t>(file);
    return header;
};

PmpObjEntry ParsePmpObjEntry(std::ifstream& file)
{
    PmpObjEntry entry;
    for (int i = 0; i < 3; i++) entry.loc[i] = ReadStream<float>(file);
    for (int i = 0; i < 3; i++) entry.rot[i] = ReadStream<float>(file);
    for (int i = 0; i < 3; i++) entry.scale[i] = ReadStream<float>(file);
    entry.offset = ReadStream<uint32_t>(file);
    entry.unk_28 = ReadStream<uint32_t>(file);
    entry.flags = ReadStream<uint16_t>(file);
    entry.unk_2E = ReadStream<uint16_t>(file);
    return entry;
}

PmpTexEntry ParsePmpTexEntry(std::ifstream& file)
{
    PmpTexEntry entry;
    entry.offset = ReadStream<uint32_t>(file);
    file.read(entry.name, 0xC);
    for (int i = 0; i < 4; i++) entry.unk_10[i] = ReadStream<uint32_t>(file);
    return entry;
}

PmpFile PmpFile::ReadPmpFile(std::ifstream& file, std::streamoff base)
{
    PmpFile pmp;
    pmp.header = ParsePmpHeader(file);
    for (int o = 0; o < pmp.header.obj_count; o++)
        pmp.objects.push_back(ParsePmpObjEntry(file));
    file.seekg(base + (std::streamoff)pmp.header.tex_index_offset);
    for (int t = 0; t < pmp.header.tex_count; t++)
        pmp.textures.push_back(ParsePmpTexEntry(file));
    for (PmpObjEntry& entry : pmp.objects)
    {
        file.seekg(base + (std::streamoff)entry.offset);
        entry.data = PmoFile::ReadPmoFile(file, base);
    }
    for (PmpTexEntry& entry : pmp.textures)
    {
        file.seekg(base + (std::streamoff)entry.offset);
        entry.data = Tm2File::ReadTm2File(file, base);
    }
    return pmp;
}

uint16_t PmpFile::objectCount()
{
    return header.obj_count;
}

uint16_t PmpFile::textureCount()
{
    return header.tex_count;
}