#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <intrin.h>
#include "OptParse.h"

using std::filesystem::path;

#define VERSION(MAJOR, MINOR, PATCH) #MAJOR ". " #MINOR "." #PATCH

struct vertex
{
    float x, y, z, w;
};

struct face
{
    int16_t a, b, c, d;
};

static_assert(sizeof(float) == 0x4);
static_assert(sizeof(uint32_t) == 0x4);
static_assert(sizeof(int16_t) == 0x2);

int stat_verts = 0;
int stat_faces = 0;

bool bcd2obj(path inFilePath, path outFilePath)
{
    std::vector<vertex> verts;
    std::vector<face> faces;

    std::ifstream inf = std::ifstream(inFilePath, std::ifstream::binary);
    
    int16_t vcount, fcount;
    uint32_t vertsPtr, facesPtr;
    inf.seekg(0x34);
    inf.read((char *)&vcount, sizeof(vcount));
    inf.read((char*)&fcount, sizeof(fcount));
    inf.read((char*)&vertsPtr, sizeof(vertsPtr));
    inf.read((char*)&facesPtr, sizeof(facesPtr));
    
    float v[4];
    inf.seekg(vertsPtr);
    for (int i = 0; i < vcount; i++)
    {
        inf.read((char*)&v, 4 * sizeof(float));
        verts.push_back({ v[0], v[1], v[2], v[3] });
    }

    int16_t f[4];
    inf.seekg(facesPtr);
    inf.seekg((4 * 0x4), std::ifstream::cur);
    for (int i = 0; i < fcount; i++)
    {
        inf.read((char*)&f, 4 * sizeof(int16_t));
        faces.push_back({ f[0], f[1], f[2], f[3] });
        inf.seekg((0x30 - 0x8), std::ifstream::cur);
    }

    inf.close();

    stat_verts = verts.size();
    stat_faces = faces.size();

    std::ofstream of = std::ofstream(outFilePath, std::ofstream::trunc);

    for (auto itr = verts.begin(); itr < verts.end(); itr++)
    {
        of << "v " << itr->x << " " << itr->y << " " << itr->z << " " << itr->w << std::endl;
    }
    for (auto itr = faces.begin(); itr < faces.end(); itr++)
    {
        of << "f " << itr->a + 1 << " " << itr->b + 1 << " " << itr->c + 1;
        if (itr->d != -1)
        {
            of << " " << itr->d + 1;
        }
        of << std::endl;
    }

    of.close();

    return true;
}

char* inPath = nullptr;
char* outPath = nullptr;

CLI_OPT g_optList[] =
{
    {
        "i", "in", OPT_STRING, &inPath
    },
    {
        "o", "out", OPT_STRING, &outPath
    }
};

const int g_optCount = sizeof(g_optList) / sizeof(CLI_OPT);

void Usage()
{
    std::cout << "Usage: bcd2obj <input_path> [output_path]" << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "bcd2obj by Thamstras. Version: " << VERSION(0, 1, 0) << std::endl;
    
    //ParseOpts(g_optList, g_optCount, argc, argv);
    
    if (inPath == nullptr && outPath == nullptr)
    {
        if (argc >= 2 && argv[1][0] != '-')
        {
            inPath = argv[1];

            if (argc == 3 && argv[2][0] != '-')
            {
                outPath = argv[2];
            }
        }
    }
    
    if (inPath == nullptr)
    {
        std::cerr << "No Input Path!" << std::endl;
        Usage();
        return -1;
    }
    
    path inputFSPath = path(inPath);
    path outputFSPath;
    
    if (outPath == nullptr)
    {
        outputFSPath = inputFSPath;
        outputFSPath.replace_extension(".obj");
    }
    else
    {
        outputFSPath = outPath;
    }

    if (!std::filesystem::exists(inputFSPath) || !std::filesystem::is_regular_file(inputFSPath))
    {
        std::cerr << "Failed to find input file!" << std::endl;
        Usage();
        return -1;
    }

    std::cout << "INPUT: " << inputFSPath << std::endl;
    std::cout << "OUTPUT: " << outputFSPath << std::endl;
    
    std::cout << "Converting... ";
    bcd2obj(inputFSPath, outputFSPath);
    std::cout << "DONE!" << std::endl;

    std::cout << "Read " << stat_verts << " verts and " << stat_faces << " faces." << std::endl;

    return 0;
}
