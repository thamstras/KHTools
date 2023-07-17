/*
    DDDTex by Thamstras (2021)
    This code (DDDTex.cpp and structs.h) is released under the MIT License.
    Other code files have their own licenses.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "structs.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
//#include "ETC1.cpp"
void ConvertETC1(unsigned int* dataOut, unsigned int* dataOutSize, unsigned int* dataIn, unsigned short width, unsigned short height, bool alpha);
#include "rg_etc1.h"

// compile DEBUG with cl /Od /EHsc /FeDDDTex /std:c++17 /Zi /MTd /W3 DDDTex.cpp ETC1.cpp rg_etc1.cpp
// compile RELEASE with cl /O2 /EHsc /FeDDDTex /std:c++17 /MT /W3 DDDTex.cpp ETC1.cpp rg_etc1.cpp

constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;

void PrintHelp()
{
    std::cerr << "DDDTex version " << VERSION_MAJOR << "." << VERSION_MINOR << " by Thamstras" << std::endl;
    std::cerr << "Usage: DDDTex.exe <file> [options]" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "\t-o: Override output file name (defaults to same name as input)" << std::endl;
    std::cerr << "\t    Note files are output as .png" << std::endl;
    exit(EXIT_FAILURE);
}

std::string g_inputFile;
std::string g_outputFile;

const char * TexFormatString(TEX_FORMAT format)
{
    switch (format)
    {
    case RGBA_8888: return "RGBA_8888";
    case RGB_888: return "RGB_888";
    case RGBA_5551: return "RGBA_5551";
    case RGB_565: return "RGB_565";
    case RGBA_4444: return "RGBA_4444";
    case LA8: return "LA8";
    case HILO8: return "HILO8";
    case L8: return "L8";
    case A8: return "A8";
    case LA4: return "LA4";
    case L4: return "L4";
    case A4: return "A4";
    case ETC1: return "ETC1";
    case ETC1A4: return "ETC1A4";
    default: return "";
    }
}

uint16 MortonCode(uint8 x, uint8 y)
{
    /*
              00  01  02  03  04  05  06  07

        00    00  01  04  05  10  11  14  15
        01    02  03  06  07  12  13  16  17
        02    08  09  0C  0D  18  19  1C  1D  
        03    0A  0B  0E  0F  1A  1B  1E  1F
        04    20  21  24  25  30  31  34  35
        05    22  23  26  27  32  33  36  37
        06    28  29  2C  2D  38  39  3C  3D
        07    2A  2B  2E  2F  3A  3B  3E  3F
    */

   uint16 z = 0;
   for (int i = 0; i < 8; i++)
   {
       z |= (x & (1U << i)) << i | (y & (1U << i)) << (i + 1);
   }
   return z;
}

// Converts WxH tiled texture data into linear texture data
void Untile(uint8 blockWidth, uint8 blockHeight, bool morton, uint8* inData, uint8* outData, int imgWidth, int imgHeight, int pixelSize)
{
    int blockRowSize = blockWidth * pixelSize;
    int blockSize = blockRowSize * blockHeight;
    uint8 * blockBuffer = new uint8[blockSize];
    uint8 * unswizzleBuffer = new uint8[blockSize];
    
    for (int srcPxY = 0; srcPxY < imgHeight; srcPxY += blockHeight)
    for (int srcPxX = 0; srcPxX < imgWidth;  srcPxX += blockWidth)
    {
        int blkY = srcPxY / blockHeight;
        int blkX = srcPxX / blockWidth;
        int blkNum = blkY * (imgWidth / blockWidth) + blkX;
        int srcPtr = blkNum * blockSize;
        memcpy(blockBuffer, inData + srcPtr, blockSize);

        if (morton)
        {
            for (uint8 swzy = 0; swzy < blockHeight; swzy++)
            {
                for (uint8 swzx = 0; swzx < blockWidth; swzx++)
                {
                    uint32 ptr = MortonCode(swzx, swzy);
                    for (int b = 0; b < pixelSize; b++)
                        unswizzleBuffer[(swzy * blockRowSize + swzx * pixelSize) + b] = blockBuffer[(ptr * pixelSize) + b];
                }
            }
            memcpy(blockBuffer, unswizzleBuffer, blockSize);
        }

        for (int dstPxY = 0; dstPxY < blockHeight; dstPxY++)
        {
            int blockBufPtr = dstPxY * blockRowSize;
            int dstPtr = ((srcPxY + dstPxY) * imgWidth * pixelSize) + (srcPxX * pixelSize);
            memcpy(&(outData[dstPtr]), &(blockBuffer[blockBufPtr]), blockRowSize);
        }
    }

    delete[] blockBuffer;
    delete[] unswizzleBuffer;
}

/*void DecodeETC1A4(uint8* inData, uint32 sz_inData, uint16 width, uint16 height, uint8** outData, uint32* sz_outData)
{
    uint64_t currData, currAlpha;
    uint32_t pixelBuffer[4 * 4];
    //uint32_t blockBuffer[8 * 8];
    size_t readPtr = 0, writePtr = 0;

    size_t outSize = 4 * width * height;
    uint8_t * decodeBuffer = new uint8_t[outSize];

    //__debugbreak();

    while (readPtr < sz_inData)
    {
        memcpy(&currAlpha, &(inData[readPtr]), sizeof(uint64_t));
        readPtr += sizeof(uint64_t);

        memcpy(&currData, &(inData[readPtr]), sizeof(uint64_t));
        readPtr += sizeof(uint64_t);

        //currAlpha = _byteswap_uint64(currAlpha);
        currData = _byteswap_uint64(currData);
        rg_etc1::unpack_etc1_block(&currData, pixelBuffer, false);

        for (int p = 0; p < (4 * 4); p++)
        {
            decodeBuffer[writePtr++] = (pixelBuffer[p] & 0x000000FF) >> 0;
            decodeBuffer[writePtr++] = (pixelBuffer[p] & 0x0000FF00) >> 8;
            decodeBuffer[writePtr++] = (pixelBuffer[p] & 0x00FF0000) >> 16;
            uint64_t alpha = (currAlpha >> (4 * p)) & 0xF;
            alpha = alpha | alpha << 4;
            decodeBuffer[writePtr++] = alpha;
        }

    }

    uint8 * outputBuffer = new uint8_t[outSize];
    Untile(8, 8, true, decodeBuffer, outputBuffer, width, height, 4);
    *outData = outputBuffer;
    *sz_outData = outSize;

    delete[] decodeBuffer;
}*/

void DecodeETC1A4_2(uint8* inData, uint32 sz_inData, uint16 width, uint16 height, uint8** outData, uint32* sz_outData)
{
    uint64_t currData, currAlpha;
    uint32_t pixelBuffer[4 * 4];

    size_t readPtr = 0;

    size_t outSize = 4 * width * height;
    uint8_t * decodeBuffer = new uint8_t[outSize];

    for (int imgY = 0; imgY < height; imgY += 8)
    {
        for (int imgX = 0; imgX < width; imgX += 8)
        {
            for (int z = 0; z < 4; z++)
            {
                memcpy(&currAlpha, &(inData[readPtr]), sizeof(uint64_t));
                readPtr += sizeof(uint64_t);

                memcpy(&currData, &(inData[readPtr]), sizeof(uint64_t));
                readPtr += sizeof(uint64_t);

                currData = _byteswap_uint64(currData);
                rg_etc1::unpack_etc1_block(&currData, pixelBuffer, false);

                for (int p = 0; p < 4 * 4; p++)
                {
                    // For some reason each block of 4x4 alpha pixels needs to be flipped around it's diagonal.
                    int x = p % 4, y = p / 4;
                    int ap = x * 4 + y;
                    uint64_t alpha = (currAlpha >> (4 * ap)) & 0xF;
                    alpha = alpha | alpha << 4;
                    pixelBuffer[p] = (pixelBuffer[p] & 0x00FFFFFF) | ((alpha & 0xFF) << 24);
                }

                int outY = imgY, outX = imgX;
                if (z == 1 || z == 3) outX += 4;
                if (z == 2 || z == 3) outY += 4;

                size_t blkPtr = 0;
                for (int y = outY; y < outY + 4; y++)
                {
                    for (int x = outX; x < outX + 4; x++)
                    {
                        decodeBuffer[4 * (y * width + x) + 0] = (pixelBuffer[blkPtr] & 0x000000FF) >> 0;
                        decodeBuffer[4 * (y * width + x) + 1] = (pixelBuffer[blkPtr] & 0x0000FF00) >> 8;
                        decodeBuffer[4 * (y * width + x) + 2] = (pixelBuffer[blkPtr] & 0x00FF0000) >> 16;
                        decodeBuffer[4 * (y * width + x) + 3] = (pixelBuffer[blkPtr] & 0xFF000000) >> 24;
                        blkPtr += 1;
                    }
                }
            }
        }
    }

    uint8 * outputBuffer = new uint8_t[outSize];
    //Untile(8, 8, false, decodeBuffer, outputBuffer, width, height, 4);
    memcpy(outputBuffer, decodeBuffer, outSize);
    *outData = outputBuffer;
    *sz_outData = outSize;

    delete[] decodeBuffer;
}

void DecodeRGB888(uint8* inData, uint32 sz_inData, uint16 width, uint16 height, uint8** outData, uint32* sz_outData)
{
    if (sz_inData != (width * height * 3))
    {
        std::cerr << "FORMAT SIZE ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint8* inputBuffer = new uint8[sz_inData];
    Untile(8, 8, true, inData, inputBuffer, width, height, 3);
    
    int outSize = 4 * width * height;
    uint8* decodeBuffer = new uint8[outSize];

    for (int py = 0; py < height; py++)
    {
        for (int px = 0; px < width; px++)
        {
            int srcPtr = (py * width + px) * 3;
            uint32 inputPixel = inputBuffer[srcPtr] | (inputBuffer[srcPtr + 1] << 8) | (inputBuffer[srcPtr + 2] << 12);
            int dstPtr = (py * width + px) * 4;
            decodeBuffer[dstPtr + 0] = ((inputPixel & 0xFF0000) >> 16);
            decodeBuffer[dstPtr + 1] = ((inputPixel & 0x00FF00) >> 8);
            decodeBuffer[dstPtr + 2] = ((inputPixel & 0x0000FF) >> 0);
            decodeBuffer[dstPtr + 3] = 0xFF;
        }
    }

    *outData = decodeBuffer;
    *sz_outData = 4 * width * height;

    delete[] inputBuffer;
}

void DecodeRGBA4444(uint8* inData, uint32 sz_inData, uint16 width, uint16 height, uint8** outData, uint32* sz_outData)
{
    if (sz_inData != (width * height * 2))
    {
        std::cerr << "FORMAT SIZE ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint8* inputBuffer = new uint8[sz_inData];
    Untile(8, 8, true, inData, inputBuffer, width, height, 2);
    
    int outSize = 4 * width * height;
    uint8* decodeBuffer = new uint8[outSize];

    for (int py = 0; py < height; py++)
    {
        for (int px = 0; px < width; px++)
        {
            int srcPtr = (py * width + px) * 2;
            uint16 inputPixel = inputBuffer[srcPtr] | (inputBuffer[srcPtr + 1] << 8);
            int dstPtr = (py * width + px) * 4;
            decodeBuffer[dstPtr + 0] = ((float)((inputPixel & 0xF000) >> 12) / 15.f) * 255.f;
            decodeBuffer[dstPtr + 1] = ((float)((inputPixel & 0x0F00) >> 8) / 15.f) * 255.f;
            decodeBuffer[dstPtr + 2] = ((float)((inputPixel & 0x00F0) >> 4) / 15.f) * 255.f;
            decodeBuffer[dstPtr + 3] = ((float)((inputPixel & 0x000F) >> 0) / 15.f) * 255.f;
        }
    }

    *outData = decodeBuffer;
    *sz_outData = 4 * width * height;

    delete[] inputBuffer;
}

void DecodeRGBA5551(uint8* inData, uint32 sz_inData, uint16 width, uint16 height, uint8** outData, uint32* sz_outData)
{
    if (sz_inData != (width * height * 2))
    {
        std::cerr << "FORMAT SIZE ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint8* inputBuffer = new uint8[sz_inData];
    Untile(8, 8, true, inData, inputBuffer, width, height, 2);
    
    int outSize = 4 * width * height;
    uint8* decodeBuffer = new uint8[outSize];

    for (int py = 0; py < height; py++)
    {
        for (int px = 0; px < width; px++)
        {
            int srcPtr = (py * width + px) * 2;
            uint16 inputPixel = inputBuffer[srcPtr] | (inputBuffer[srcPtr + 1] << 8);
            int dstPtr = (py * width + px) * 4;
            decodeBuffer[dstPtr + 0] = ((float)((inputPixel & 0xF800) >> 11) / 31.f) * 255.f;
            decodeBuffer[dstPtr + 1] = ((float)((inputPixel & 0x07C0) >> 6) / 31.f) * 255.f;
            decodeBuffer[dstPtr + 2] = ((float)((inputPixel & 0x003E) >> 1) / 31.f) * 255.f;
            decodeBuffer[dstPtr + 3] = 0xFF * (inputPixel & 0x0001);
        }
    }

    *outData = decodeBuffer;
    *sz_outData = 4 * width * height;

    delete[] inputBuffer;
}

void DecodeRGB565(uint8* inData, uint32 sz_inData, uint16 width, uint16 height, uint8** outData, uint32* sz_outData)
{
    if (sz_inData != (width * height * 2))
    {
        std::cerr << "FORMAT SIZE ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint8* inputBuffer = new uint8[sz_inData];
    Untile(8, 8, true, inData, inputBuffer, width, height, 2);
    
    int outSize = 4 * width * height;
    uint8* decodeBuffer = new uint8[outSize];

    for (int py = 0; py < height; py++)
    {
        for (int px = 0; px < width; px++)
        {
            int srcPtr = (py * width + px) * 2;
            uint16 inputPixel = inputBuffer[srcPtr] | (inputBuffer[srcPtr + 1] << 8);
            int dstPtr = (py * width + px) * 4;
            decodeBuffer[dstPtr + 0] = ((float)((inputPixel & 0xF800) >> 11) / 31.f) * 255.f;
            decodeBuffer[dstPtr + 1] = ((float)((inputPixel & 0x07E0) >> 5) / 63.f) * 255.f;
            decodeBuffer[dstPtr + 2] = ((float)((inputPixel & 0x001F) >> 0) / 31.f) * 255.f;
            decodeBuffer[dstPtr + 3] = 0xFF;
        }
    }

    *outData = decodeBuffer;
    *sz_outData = 4 * width * height;

    delete[] inputBuffer;
}

void DumpCTRT(FILE * fp, std::string outputFile)
{
    CTRT_HEAD header;
    fread(&header, sizeof(CTRT_HEAD), 1, fp);
    if (memcmp(&header.magic, "CTRT", 4) != 0)
    {
        std::cerr << "MAGIC FAIL!" << std::endl;
        return;
    }

    std::cout << "Width: " << header.width << " Height: " << header.height << std::endl;
    std::cout << "Format: " << TexFormatString(header.format) << std::endl;

    uint8 * encodedData;
    uint32 sz_encodedData;

    sz_encodedData = header.sz_data;
    encodedData = new uint8[sz_encodedData];
    fseek(fp, header.p_data, SEEK_SET);
    fread(encodedData, sz_encodedData, 1, fp);

    uint8* decodedData;
    uint32 sz_decodedData;

    switch (header.format)
    {
        case RGBA_8888:
        {
            decodedData = new uint8[sz_encodedData];
            sz_decodedData = sz_encodedData;
            Untile(8, 8, true, encodedData, decodedData, header.width, header.height, 4);
            break;
        }
        case RGB_888:
        {
            DecodeRGB888(encodedData, sz_encodedData, header.width, header.height, &decodedData, &sz_decodedData);
            break;
        }
        case RGBA_5551:
        {
            DecodeRGBA5551(encodedData, sz_encodedData, header.width, header.height, &decodedData, &sz_decodedData);
            break;
        }
        case RGB_565:
        {
            DecodeRGB565(encodedData, sz_encodedData, header.width, header.height, &decodedData, &sz_decodedData);
            break;
        }
        case RGBA_4444:
        {
            DecodeRGBA4444(encodedData, sz_encodedData, header.width, header.height, &decodedData, &sz_decodedData);
            break;
        }
        case ETC1:
        {
            decodedData = new uint8[sizeof(uint32) * header.width * header.height];
            sz_decodedData = sizeof(uint32) * header.width * header.height;
            ConvertETC1((uint32*)decodedData, &sz_decodedData, (uint32*)encodedData, header.width, header.height, false);
            break;
        }
        case ETC1A4:
        {
            //decodedData = new uint8[sizeof(uint32) * header.width * header.height];
            //sz_decodedData = sizeof(uint32) * header.width * header.height;
            //ConvertETC1((uint32*)decodedData, &sz_decodedData, (uint32*)encodedData, header.width, header.height, true);
            DecodeETC1A4_2(encodedData, sz_encodedData, header.width, header.height, &decodedData, &sz_decodedData);
            break;
        }
        default:
            std::cerr << "This format is not yet supported. Sorry." << std::endl;
            return;
    }

    stbi_write_png(outputFile.c_str(), header.width, header.height, 4, decodedData, 0);
    //stbi_write_bmp(outputFile.c_str(), header.width, header.height, 4, decodedData);
    std::cout << "Wrote " << outputFile << std::endl;
    delete[] decodedData;
}

int main(int argc, char ** argv)
{
    if (argc < 2) 
    {
        PrintHelp();
        return -1;
    }
    
    g_inputFile = std::string(argv[1]);
    
    g_outputFile = g_inputFile;
    
    if (argc > 2)
    {
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "-o") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::cerr << "-o requires a filename!";
                    PrintHelp();
                    return -1;
                }
                g_outputFile = std::string(argv[++i]);
            }
        }
    }

    std::filesystem::path inPath = std::filesystem::path(g_inputFile);
    if (!std::filesystem::exists(inPath))
    {
        std::cerr << "Input file doesn't exist!" << std::endl;
        return -1;
    }
    std::filesystem::path outPath = std::filesystem::path(g_outputFile);
    outPath = outPath.replace_extension(".png");
    g_outputFile = outPath.string();

    FILE * inFP = fopen(g_inputFile.c_str(), "rb");
    DumpCTRT(inFP, g_outputFile);
}