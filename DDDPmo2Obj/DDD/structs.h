#include "Utils/BinaryTypes.h"

namespace DreamDropDistance::internal
{
    #pragma pack(push, 1)

    struct FVector2
    {
        float x, y;
    };

    struct FVector3
    {
        float x, y, z;
    };

    struct FVector4
    {
        float x, y, z, w;
    };

    struct PMO_FILE_HEADER
    {
        uint32 ident;
        uint8 num;
        uint8 group;
        uint8 ver;
        uint8 pad0;
        uint8 texCount;
        uint8 pad1;
        uint16 flag;
        uint32 p_skeleton;
    };

    struct PMO_MODEL_HEADER
    {
        uint32 p_geom;
        uint16 triCount;
        uint16 vertCount;
        float modelScale;
        uint32 p_geom2;
        FVector4 bounds[8];
    };

    struct PMO_TEX_INFO
    {
        uint32 p_tex;
        char name[12];
        float scrollU;
        float scrollV;
        uint32 pad0[2];
    };

    struct DDD_GEOM_HEADER
    {
        uint32 p_attribs1;
        uint32 p_attribs2;
        uint32 vertCount;
        uint32 unk_0C;
        uint32 attribs1Count;
        uint32 attribs2Count;
        uint32 p_data;
        uint32 sz_data;
        uint32 unk_20;
        uint32 unk_24[7];   // not seen used. possibly padding.
        uint32 unk_40;
    };

    struct SUB_BUFFER_ATTRIBS
    {
        uint16 vertCount;
        uint8 texId;
        uint8 stride;
        uint8 bwType;
        uint8 unk_05;
        uint8 skinned;
        uint8 indType;
        uint8 unk_08;
        uint16 unk_09;
        uint8 unk_0B;
        uint8 boneMap[12];
    };

    #pragma pack(pop)
}