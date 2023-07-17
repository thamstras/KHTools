#include "structs.h"
#include "Utils/BinaryReader.h"
#include "Utils/MagicCode.h"
#include "pmo.h"
#include <vector>

#include "DDD/structs.h"

namespace DreamDropDistance
{
    using internal::PMO_FILE_HEADER;
    using internal::PMO_MODEL_HEADER;
    using internal::PMO_TEX_INFO;
    using internal::DDD_GEOM_HEADER;
    using internal::SUB_BUFFER_ATTRIBS;

    PMO* PMO::PmoFromFile(std::string path)
    {
        BinaryReader reader = BinaryReader(path);
        if (reader.IsOpen())
        {
            return ReadPmo(reader);
        }
        else
        {
            // TODO: Error
            return nullptr;
        }
    }

    PMO* PMO::ReadPmo(BinaryReader& reader)
    {
        uint32 pmo_base = (uint32)reader.Tell();
        PMO_FILE_HEADER file_header = ReadFileHeader(reader);
        if (file_header.ident != MagicCode('P', 'M', 'O', '\0') || file_header.ver != 4)
        {
            // TODO: Error
        }

        PMO_MODEL_HEADER model_header = ReadModelHeader(reader);

        std::vector<PMO_TEX_INFO> textureList = std::vector<PMO_TEX_INFO>();
        for (int i = 0; i < file_header.texCount; i++) textureList.push_back(ReadTexInfo(reader));

        uint32 poly_offset = 0;
        if (model_header.p_geom != 0) poly_offset = model_header.p_geom;
        else if (model_header.p_geom2 != 0) poly_offset = model_header.p_geom2;

        if (poly_offset == 0)
        {
            // TODO: Error
        }

        reader.Seek(pmo_base + poly_offset);
        DDD_GEOM_HEADER geom_header = ReadGeomHeader(reader);

        // TODO: Read Name here

        std::vector<SUB_BUFFER_ATTRIBS> subBufferAttribs1 = std::vector<SUB_BUFFER_ATTRIBS>();
        reader.Seek(pmo_base + geom_header.p_attribs1);
        for (int i = 0; i < geom_header.attribs1Count; i++) subBufferAttribs1.push_back(ReadSubBufferAttribs(reader));

        std::vector<SUB_BUFFER_ATTRIBS> subBufferAttribs2 = std::vector<SUB_BUFFER_ATTRIBS>();
        reader.Seek(pmo_base + geom_header.p_attribs2);
        for (int i = 0; i < geom_header.attribs2Count; i++) subBufferAttribs2.push_back(ReadSubBufferAttribs(reader));

        uint32 vertex_buffer_base = pmo_base + geom_header.p_data;

        for (int i = 0; i < geom_header.attribs1Count; i++)
        {
            reader.Seek(vertex_buffer_base);
            SUB_BUFFER_ATTRIBS& attribs = subBufferAttribs1.at(i);
            // ReadSubBuffer(reader, attribs);
            vertex_buffer_base += attribs.stride * attribs.vertCount;
        }

        for (int i = 0; i < geom_header.attribs2Count; i++)
        {
            reader.Seek(vertex_buffer_base);
            SUB_BUFFER_ATTRIBS& attribs = subBufferAttribs2.at(i);
            // ReadSubBuffer(reader, attribs);
            vertex_buffer_base += attribs.stride * attribs.vertCount;
        }

        // TODO: Finish
    }

    PMO_FILE_HEADER PMO::ReadFileHeader(BinaryReader& reader)
    {
        PMO_FILE_HEADER header;
        header.ident = reader.ReadU32();
        header.num = reader.ReadU8();
        header.group = reader.ReadU8();
        header.ver = reader.ReadU8();
        header.pad0 = reader.ReadU8();
        header.texCount = reader.ReadU8();
        header.pad1 = reader.ReadU8();
        header.flag = reader.ReadU16();
        header.p_skeleton = reader.ReadU32();
        return header;
    }

    PMO_MODEL_HEADER PMO::ReadModelHeader(BinaryReader& reader)
    {
        PMO_MODEL_HEADER header;
        header.p_geom = reader.ReadU32();
        header.triCount = reader.ReadU16();
        header.vertCount = reader.ReadU16();
        header.modelScale = reader.ReadFloat();
        header.p_geom2 = reader.ReadU32();
        for (int i = 0; i < 8; i++)
        {
            // TODO: read func for fvectors
            header.bounds[i].x = reader.ReadFloat();
            header.bounds[i].y = reader.ReadFloat();
            header.bounds[i].z = reader.ReadFloat();
            header.bounds[i].w = reader.ReadFloat();
        }
        return header;
    }

    PMO_TEX_INFO PMO::ReadTexInfo(BinaryReader& reader)
    {
        PMO_TEX_INFO info;
        info.p_tex = reader.ReadU32();
        for (int c = 0; c < 12; c++)
            info.name[c] = reader.ReadU8();
        info.scrollU = reader.ReadFloat();
        info.scrollV = reader.ReadFloat();
        info.pad0[0] = reader.ReadU32();
        info.pad0[1] = reader.ReadU32();
        return info;
    }

    DDD_GEOM_HEADER PMO::ReadGeomHeader(BinaryReader& reader)
    {
        DDD_GEOM_HEADER header;
        header.p_attribs1 = reader.ReadU32();
        header.p_attribs2 = reader.ReadU32();
        header.vertCount = reader.ReadU32();
        header.unk_0C = reader.ReadU32();
        header.attribs1Count = reader.ReadU32();
        header.attribs2Count = reader.ReadU32();
        header.p_data = reader.ReadU32();
        header.sz_data = reader.ReadU32();
        header.unk_20 = reader.ReadU32();
        for (int i = 0; i < 7; i++)
            header.unk_24[i] = reader.ReadU32();
        header.unk_40 = reader.ReadU32();
        return header;
    }

    SUB_BUFFER_ATTRIBS PMO::ReadSubBufferAttribs(BinaryReader& reader)
    {
        SUB_BUFFER_ATTRIBS attribs;
        attribs.vertCount = reader.ReadU16();
        attribs.texId = reader.ReadU8();
        attribs.stride = reader.ReadU8();
        attribs.bwType = reader.ReadU8();
        attribs.unk_05 = reader.ReadU8();
        attribs.skinned = reader.ReadU8();
        attribs.indType = reader.ReadU8();
        attribs.unk_08 = reader.ReadU8();
        attribs.unk_09 = reader.ReadU16();
        attribs.unk_0B = reader.ReadU8();
        for (int i = 0; i < 12; i++)
            attribs.boneMap[i] = reader.ReadU8();
        return attribs;
    }
}