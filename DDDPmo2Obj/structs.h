#include <cstdint>
#include <string>
#include <vector>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint8 ubyte;
typedef int8 byte;

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
    uint8 unk_10;
    uint8 boneMap[12];
};

enum TEX_FORMAT : uint32
{
    RGBA_8888 = 0,
    RGB_888 = 1,
    RGBA_5551 = 2,
    RGB_565 = 3,
    RGBA_4444 = 4,
    LA8 = 5,
    HILO8 = 6,
    L8 = 7,
    A8 = 8,
    LA4 = 9,
    L4 = 10,
    A4 = 11,
    ETC1 = 12,
    ETC1A4 = 13
};
    

struct CTT_HEADER
{
    uint32 magic;
    uint32 a;
    uint32 b;
    uint32 p_data;
    uint32 d;
    uint32 sz_data;
    uint32 f;
    TEX_FORMAT format;
    uint16 width;
    uint16 height;
    uint32 i;
};

struct Vertex
{
    FVector2 tex;
    FVector3 pos;
    float weights[4];
    uint8 bones[4];
};

struct MeshSection
{
    bool smoothShade;
    std::string materialName;
    std::vector<Vertex> verts;
    std::vector<uint32> indices;
};

struct Mesh
{
    std::string name;
    std::vector<MeshSection> groups;
    std::vector<std::string> materialNames;
};

#pragma pack(pop)