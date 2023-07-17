#include <cstdint>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#pragma pack(push, 1)

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

struct CTRT_HEAD
{
    uint32 magic;
    uint32 unk_04;
    uint32 unk_08;
    uint32 p_data;
    uint32 unk_10;
    uint32 sz_data;
    uint32 unk_18;
    TEX_FORMAT format;
    uint16 width;
    uint16 height;
    uint32 unk_24;
};

#pragma pack(pop)