/*
    DDDPmo2Obj by Thamstras (2021)
    This code (main.cpp and structs.h) is released under the MIT License.
    Other code files have their own licenses.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "structs.h"
#include <string>
#include <iostream>
#include <cstdio>
//#include <filesystem>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iomanip>

// compile DEBUG with cl /Od /EHsc /FeDDDPmo2Obj /std:c++17 /Zi /MTd /W3 main.cpp
// compile RELEASE with cl /O2 /EHsc /FeDDDPmo2Obj /std:c++17 /MT /W3 main.cpp

constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;

void PrintHelp()
{
    std::cerr << "DDDPmo2Obj version " << VERSION_MAJOR << "." << VERSION_MINOR << " by Thamstras" << std::endl;
    std::cerr << "Usage: DDDPmo2Obj.exe <file>" << std::endl;
    exit(EXIT_FAILURE);
}

void WriteOBJ(Mesh mesh)
{
    std::fstream mtlfs = std::fstream(mesh.name + ".mtl", std::fstream::out | std::fstream::binary);
    for each (auto& str in mesh.materialNames)
    {
        mtlfs << "newmtl " << str << std::endl;
        mtlfs << "Ka 1.000 1.000 1.000" << std::endl;
        mtlfs << "Kd 1.000 1.000 1.000" << std::endl;
        mtlfs << "Ks 0.000 0.000 0.000" << std::endl;
        mtlfs << "Ns 0.000" << std::endl;
        mtlfs << "map_Kd " << str << ".png" << std::endl;
        mtlfs << "map_Ka " << str << ".png" << std::endl;
        mtlfs << std::endl;
    }
    mtlfs.close();
    
    std::string outName = mesh.name + ".obj";
    std::fstream fs = std::fstream(outName, std::fstream::out | std::fstream::binary);
    //fs << "o " << mesh.name << std::endl;
    fs << "mtllib " << mesh.name << ".mtl" << std::endl;
    int vbase = 1;
    for (int s = 0; s < mesh.groups.size(); s++)
    {
        MeshSection section = mesh.groups[s];
        //fs << "g " << s << std::endl;
        fs << "usemtl " << section.materialName << std::endl;
        /*if (section.smoothShade)
            fs << "s 1" << std::endl;
        else
            fs << "s 0" << std::endl;
        fs << std::endl;*/
        for (int v = 0; v < section.verts.size(); v++)
        {
            Vertex vert = section.verts[v];
            if (std::isnan(vert.pos.x) || std::isnan(vert.pos.y) || std::isnan(vert.pos.z)) continue;
            if (std::isinf(vert.pos.x) || std::isinf(vert.pos.y) || std::isinf(vert.pos.z)) continue;
            fs << "v " << vert.pos.x << " " << vert.pos.y << " " << vert.pos.z << std::endl;
            fs << "vt " << vert.tex.x << " " << vert.tex.y << std::endl;
        }
        fs << std::endl;
        for (int i = 0; i < section.indices.size(); i+= 3)
        {
            fs << "f " << vbase + section.indices[i] << "/" << vbase + section.indices[i]
                << " " << vbase + section.indices[i + 2] << "/" << vbase + section.indices[i + 2]
                << " " << vbase + section.indices[i + 1] << "/" << vbase + section.indices[i + 1]
                << std::endl;
        }
        fs << std::endl;

        vbase += section.verts.size();
    }
    fs.close();
}

void DumpCTT(FILE* fp, std::string name)
{
    long cttBase = ftell(fp);
    
    CTT_HEADER cttHeader;
    fread(&cttHeader, sizeof(CTT_HEADER), 1, fp);
    if (memcmp(&cttHeader.magic, "CTRT", 4) != 0)
    {
        std::cerr << "CTT " << name << " IS CORRUPT" << std::endl;
        return;
    }

    uint8 * data = new uint8[cttHeader.sz_data];
    fseek(fp, cttBase + cttHeader.p_data, SEEK_SET);
    fread(data, 1, cttHeader.sz_data, fp);

    std::string outName = name + ".ctt";
    FILE * fpout = fopen(outName.c_str(), "wb");
    if (fpout)
    {
        fwrite(&cttHeader, sizeof(CTT_HEADER), 1, fpout);
        fseek(fpout, cttHeader.p_data, SEEK_SET);
        fwrite(data, 1, cttHeader.sz_data, fpout);
        fclose(fpout);

        std::string decodeCommand = std::string("DDDTex ") + outName;
        system(decodeCommand.c_str());
    }
    else
    {
        std::cerr << "COULDN'T WRITE " << outName << std::endl;
    }

}

void DumpPMO2(FILE* fp)
{
    long pmoBase = ftell(fp);
    
    PMO_FILE_HEADER fileHeader;
    fread(&fileHeader, sizeof(fileHeader), 1, fp);
    if (memcmp(&fileHeader.ident, "PMO\0", 4) != 0)
    {
        std::cout << "MAGIC FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (fileHeader.ver != 4)
    {
        std::cout << "FILE VERSION INCORRECT" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    PMO_MODEL_HEADER modelHeader;
    fread(&modelHeader, sizeof(modelHeader), 1, fp);
    
    std::cout << std::hex;
    std::cout << std::setprecision(4) << std::showpos << std::showpoint << std::setfill('0');
    std::cout << "PMO HEADER" << std::endl;
    std::cout << "ident:      " << std::setw(8) << fileHeader.ident << std::endl;
    std::cout << "num:        " << std::setw(2) << (uint32)fileHeader.num << std::endl;
    std::cout << "group:      " << std::setw(2) << (uint32)fileHeader.group << std::endl;
    std::cout << "ver:        " << std::setw(2) << (uint32)fileHeader.ver << std::endl;
    std::cout << "pad0:       " << std::setw(2) << (uint32)fileHeader.pad0 << std::endl;
    std::cout << "texCount:   " << std::setw(2) << (uint32)fileHeader.texCount << std::endl;
    std::cout << "pad1:       " << std::setw(2) << (uint32)fileHeader.pad1 << std::endl;
    std::cout << "flag:       " << std::setw(4) << fileHeader.flag << std::endl;
    std::cout << "p_skeleton: " << std::setw(8) << fileHeader.p_skeleton << std::endl;
    std::cout << std::endl;
    std::cout << "p_geom:     " << std::setw(8) << modelHeader.p_geom << std::endl;
    std::cout << "triCount:   " << std::setw(4) << modelHeader.triCount << std::endl;
    std::cout << "vertCount:  " << std::setw(4) << modelHeader.vertCount << std::endl;
    std::cout << "modelScale: " << modelHeader.modelScale << std::endl;
    std::cout << "p_geom2:    " << std::setw(8) << modelHeader.p_geom2 << std::endl;
    std::cout << "bounds:     " << modelHeader.bounds[0].x << " " << modelHeader.bounds[0].y << " " << modelHeader.bounds[0].z << " " << modelHeader.bounds[0].w << std::endl;
    std::cout << "            " << modelHeader.bounds[1].x << " " << modelHeader.bounds[1].y << " " << modelHeader.bounds[1].z << " " << modelHeader.bounds[1].w << std::endl;
    std::cout << "            " << modelHeader.bounds[2].x << " " << modelHeader.bounds[2].y << " " << modelHeader.bounds[2].z << " " << modelHeader.bounds[2].w << std::endl;
    std::cout << "            " << modelHeader.bounds[3].x << " " << modelHeader.bounds[3].y << " " << modelHeader.bounds[3].z << " " << modelHeader.bounds[3].w << std::endl;
    std::cout << "            " << modelHeader.bounds[4].x << " " << modelHeader.bounds[4].y << " " << modelHeader.bounds[4].z << " " << modelHeader.bounds[4].w << std::endl;
    std::cout << "            " << modelHeader.bounds[5].x << " " << modelHeader.bounds[5].y << " " << modelHeader.bounds[5].z << " " << modelHeader.bounds[5].w << std::endl;
    std::cout << "            " << modelHeader.bounds[6].x << " " << modelHeader.bounds[6].y << " " << modelHeader.bounds[6].z << " " << modelHeader.bounds[6].w << std::endl;
    std::cout << "            " << modelHeader.bounds[7].x << " " << modelHeader.bounds[7].y << " " << modelHeader.bounds[7].z << " " << modelHeader.bounds[7].w << std::endl;
    std::cout << std::endl;

    uint32 target = 0;
    if (modelHeader.p_geom != 0) target = modelHeader.p_geom;
    else if (modelHeader.p_geom2 != 0) target = modelHeader.p_geom2;
    if (target == 0)
    {
        std::cout << "NO GEOM PTR" << std::endl;
        exit(EXIT_FAILURE);
    }

    PMO_TEX_INFO * texInfo = new PMO_TEX_INFO[fileHeader.texCount];
    fread(texInfo, sizeof(PMO_TEX_INFO), fileHeader.texCount, fp);

    for (int t = 0; t < fileHeader.texCount; t++)
    {
        std::cout << "TEXTURE " << t << std::endl;
        std::cout << "p_tex:   " << std::setw(8) << texInfo[t].p_tex << std::endl;
        std::cout << "name:    " << texInfo[t].name << std::endl;
        std::cout << "scrollU: " << texInfo[t].scrollU << std::endl;
        std::cout << "scrollV: " << texInfo[t].scrollV << std::endl;
        std::cout << "pad0:    " << std::setw(8) << texInfo[t].pad0[0] << " " << std::setw(8) << texInfo[t].pad0[1] << std::endl;
        std::cout << std::endl;
    }

    DDD_GEOM_HEADER geomHeader;
    fseek(fp, pmoBase + target, SEEK_SET);
    fread(&geomHeader, sizeof(geomHeader), 1, fp);

    std::cout << "GEOMETRY HEADER" << std::endl;
    std::cout << "p_attribs1:    " << std::setw(8) << geomHeader.p_attribs1 << std::endl;
    std::cout << "p_attribs2:    " << std::setw(8) << geomHeader.p_attribs2 << std::endl;
    std::cout << "vertCount:     " << std::setw(8) << geomHeader.vertCount << std::endl;
    std::cout << "unk_0C:        " << std::setw(8) << geomHeader.unk_0C << std::endl;
    std::cout << "attribs1Count: " << std::setw(8) << geomHeader.attribs1Count << std::endl;
    std::cout << "attribs2Count: " << std::setw(8) << geomHeader.attribs2Count << std::endl;
    std::cout << "p_data:        " << std::setw(8) << geomHeader.p_data << std::endl;
    std::cout << "sz_data:       " << std::setw(8) << geomHeader.sz_data << std::endl;
    std::cout << "unk_20:        " << std::setw(8) << geomHeader.unk_20 << std::endl;
    std::cout << "unk_24:        " << std::setw(8) << geomHeader.unk_24[0] << std::endl;
    std::cout << "unk_40:        " << std::setw(8) << geomHeader.unk_40 << std::endl;

    std::vector<char> name = std::vector<char>();
    do
    {
        name.push_back(fgetc(fp));
    }
    while (name.at(name.size() - 1) != '\0' && name.at(name.size() - 1) != EOF);
    if (name.at(name.size() - 1) == EOF) 
    {
        std::cout << "NAME READ ERROR/UNEXPECTED EOF" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "NAME: " << name.data() << std::endl;
    std::cout << std::endl;

    SUB_BUFFER_ATTRIBS * attribs1 = new SUB_BUFFER_ATTRIBS[geomHeader.attribs1Count];
    fseek(fp, pmoBase + geomHeader.p_attribs1, SEEK_SET);
    fread(attribs1, sizeof(SUB_BUFFER_ATTRIBS), geomHeader.attribs1Count, fp);
    
    SUB_BUFFER_ATTRIBS * attribs2 = new SUB_BUFFER_ATTRIBS[geomHeader.attribs2Count];
    fseek(fp, pmoBase + geomHeader.p_attribs2, SEEK_SET);
    fread(attribs2, sizeof(SUB_BUFFER_ATTRIBS), geomHeader.attribs2Count, fp);

    std::cout << "ATTRIBS 1" << std::endl;
    std::cout << "VertCount: " << std::setw(4) << attribs1[0].vertCount << std::endl;
    std::cout << "texId: " << std::setw(2) << (uint32)attribs1[0].texId << std::endl;
    std::cout << "stride: " << std::setw(2) << (uint32)attribs1[0].stride << std::endl;
    std::cout << "bwType: " << std::setw(2) << (uint32)attribs1[0].bwType << std::endl;
    std::cout << "unk_05: " << std::setw(2) << (uint32)attribs1[0].unk_05 << std::endl;
    std::cout << "skinned: " << std::setw(2) << (uint32)attribs1[0].skinned << std::endl;
    std::cout << "indType: " << std::setw(2) << (uint32)attribs1[0].indType << std::endl;
    std::cout << "unk_08: " << std::setw(2) << (uint32)attribs1[0].unk_08 << std::endl;
    std::cout << "unk_09: " << std::setw(4) << attribs1[0].unk_09 << std::endl;
    std::cout << "unk_10: " << std::setw(2) << (uint32)attribs1[0].unk_10 << std::endl;
    std::cout << "VertCount: " << std::setw(4) << attribs1[0].vertCount << std::endl;








}

void DumpPMO(FILE* fp)
{
    long pmoBase = ftell(fp);
    
    PMO_FILE_HEADER fileHeader;
    fread(&fileHeader, sizeof(fileHeader), 1, fp);
    if (memcmp(&fileHeader.ident, "PMO\0", 4) != 0)
    {
        std::cout << "MAGIC FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (fileHeader.ver != 4)
    {
        std::cout << "FILE VERSION INCORRECT" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << std::dec;
    std::cout << "Read version " << (int)fileHeader.ver << ". Textures: " << (int)fileHeader.texCount
        << " Flag: " << (int)fileHeader.flag << " Skeleton: " << (fileHeader.p_skeleton != 0) << std::endl;
    
    PMO_MODEL_HEADER modelHeader;
    fread(&modelHeader, sizeof(modelHeader), 1, fp);
    
    std::cout << "Read model header. Expecting " << modelHeader.vertCount << " verts and "
        << modelHeader.triCount << " tris." << std::endl;
    std::cout << "Scale: " << modelHeader.modelScale << std::endl;

    uint32 target = 0;
    if (modelHeader.p_geom != 0) target = modelHeader.p_geom;
    else if (modelHeader.p_geom2 != 0) target = modelHeader.p_geom2;
    if (target == 0)
    {
        std::cout << "NO GEOM PTR" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // TODO: Read Texture info
    PMO_TEX_INFO * texInfo = new PMO_TEX_INFO[fileHeader.texCount];
    fread(texInfo, sizeof(PMO_TEX_INFO), fileHeader.texCount, fp);

    std::cout << "TEXTURES" << std::endl;
    std::cout << std::hex;
    std::vector<std::string> matNames = std::vector<std::string>();
    for (int t = 0; t < fileHeader.texCount; t++)
    {
        char tmp[13];
        memcpy(tmp, texInfo[t].name, 12);
        tmp[12] = '\0';
        std::cout << tmp << " @ " << texInfo[t].p_tex << std::endl;
        matNames.push_back(std::string(tmp));

        fseek(fp, pmoBase + texInfo[t].p_tex, SEEK_SET);
        DumpCTT(fp, matNames[t]);
    }
    std::cout << std::dec;

    DDD_GEOM_HEADER geomHeader;
    fseek(fp, pmoBase + target, SEEK_SET);
    fread(&geomHeader, sizeof(geomHeader), 1, fp);

    std::cout << "Read Geometry Header. " << geomHeader.vertCount << " verts, "
        << geomHeader.attribs1Count << ", " << geomHeader.attribs2Count << " subbuffers." << std::endl;
    std::cout << std::hex;
    std::cout << "Vertex buffer @ " << geomHeader.p_data << ", size " << geomHeader.sz_data << " bytes." << std::endl;
    std::cout << std::dec;

    std::vector<char> name = std::vector<char>();
    do
    {
        name.push_back(fgetc(fp));
    }
    while (name.at(name.size() - 1) != '\0' && name.at(name.size() - 1) != EOF);
    if (name.at(name.size() - 1) == EOF) 
    {
        std::cout << "NAME READ ERROR/UNEXPECTED EOF" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "READ NAME: " << name.data() << std::endl;
    
    SUB_BUFFER_ATTRIBS * attribs1 = new SUB_BUFFER_ATTRIBS[geomHeader.attribs1Count];
    fseek(fp, pmoBase + geomHeader.p_attribs1, SEEK_SET);
    fread(attribs1, sizeof(SUB_BUFFER_ATTRIBS), geomHeader.attribs1Count, fp);
    
    SUB_BUFFER_ATTRIBS * attribs2 = new SUB_BUFFER_ATTRIBS[geomHeader.attribs2Count];
    fseek(fp, pmoBase + geomHeader.p_attribs2, SEEK_SET);
    fread(attribs2, sizeof(SUB_BUFFER_ATTRIBS), geomHeader.attribs2Count, fp);
    
    std::vector<MeshSection> sections = std::vector<MeshSection>();
    sections.reserve(geomHeader.attribs1Count + geomHeader.attribs2Count);

    uint32 bufferBase = pmoBase + geomHeader.p_data;
    
    std::cout << "ATTRIBS 1" << std::endl;
    
    for (int i = 0; i < geomHeader.attribs1Count; i++)
    {
        std::vector<Vertex> vertices = std::vector<Vertex>();
        std::vector<int32> ffList = std::vector<int32>();
        
        SUB_BUFFER_ATTRIBS attribs = attribs1[i];

        bool floatVert = (attribs.bwType >> 4) > 2;
        bool skinned = (attribs.bwType & 0xF) > 0;

        std::cout << "Subbuffer " << i << ", verts " << attribs.vertCount << ", " << (floatVert ? "float" : "int16")  << ", " << (skinned ? "Skinned" : "") << std::endl;

        for (int v = 0; v < attribs.vertCount; v++)
        {
            Vertex vertex;
            
            fseek(fp, bufferBase + (v * attribs.stride), SEEK_SET);
            
            uint16 uv[2];
            fread(uv, sizeof(uint16), 2, fp);
            // TODO: Confirm this divisor
            vertex.tex.x = uv[0] / 2048.f;
            //vertex.tex.y = 1.f - (uv[1] / 2048.f);
            vertex.tex.y = (uv[1] / 2048.f);
            
            uint32 ff;
            fread(&ff, sizeof(uint32), 1, fp);
            ffList.push_back(ff);
            
            if (floatVert)
            {
                float pos[3];
                fread(pos, sizeof(float), 3, fp);
                // TODO: Confirm this divisor
                vertex.pos.x = (pos[0]/30720.f) * modelHeader.modelScale;
                vertex.pos.y = (pos[1]/30720.f) * modelHeader.modelScale;
                vertex.pos.z = (pos[2]/30720.f) * modelHeader.modelScale;
            }
            else
            {
                int16 pos[3];
                fread(pos, sizeof(int16), 3, fp);
                vertex.pos.x = (pos[0]/(float)INT16_MAX) * modelHeader.modelScale;
                vertex.pos.y = (pos[1]/(float)INT16_MAX) * modelHeader.modelScale;
                vertex.pos.z = (pos[2]/(float)INT16_MAX) * modelHeader.modelScale;
            }
            
            if (skinned)
            {
                uint8 weights[4];
                uint8 bones[4];
                fread(weights, sizeof(uint8), 4, fp);
                fread(bones, sizeof(uint8), 4, fp);
                for (int b = 0; b < 4; b++)
                {
                    vertex.weights[b] = weights[b] / (float)INT8_MAX;
                    vertex.bones[b] = attribs.boneMap[bones[b] / 3]; // TODO: Why div 3?
                }
            }
            else
            {
                for (int b = 0; b < 4; b++)
                {
                    vertex.weights[b] = 0;
                    vertex.bones[b] = 0;
                }
            }

            vertices.push_back(vertex);
        }

        MeshSection section = MeshSection();
        section.verts = vertices;
        section.smoothShade = false;
        section.materialName = std::string(texInfo[attribs.texId].name);
        std::cout << section.materialName << std::endl;
        if (attribs.indType == 0x40 || attribs.indType == 0x41)
        {
            for (int v = 0; v < attribs.vertCount - 2; v++)
            {
                if (ffList[v] == 8421504 || ffList[v + 1] == 8421504 || ffList[v + 2] == 8421504)
                    continue;

                section.indices.push_back(v);
                if (v % 2 == 0)
                {
                    section.indices.push_back(v + 2);
                    section.indices.push_back(v + 1);
                }
                else
                {
                    section.indices.push_back(v + 1);
                    section.indices.push_back(v + 2);
                }
            }
        }
        else if (attribs.indType == 0x30 || attribs.indType == 0x32)
        {
            for (int v = 0; v < attribs.vertCount; v += 3)
            {
                section.indices.push_back(v);
                section.indices.push_back(v + 2);
                section.indices.push_back(v + 1);
            }
        }
        else
        {
            std::cout << std::hex;
            std::cout << "UNKOWN GEOM TYPE " << (uint32)attribs.indType << std::endl;
            std::cout << std::dec;
        }

        sections.push_back(section);

        bufferBase += attribs.stride * attribs.vertCount;
    }

    std::cout << "ATTRIBS 2" << std::endl;
    
    for (int i = 0; i < geomHeader.attribs2Count; i++)
    {
        std::vector<Vertex> vertices = std::vector<Vertex>();
        std::vector<int32> ffList = std::vector<int32>();
        
        SUB_BUFFER_ATTRIBS attribs = attribs2[i];

        bool floatVert = (attribs.bwType >> 4) > 2;
        bool skinned = (attribs.bwType & 0xF) > 0;

        std::cout << "Subbuffer " << i << ", verts " << attribs.vertCount << ", " << (floatVert ? "float" : "int16") << ", " << (skinned ? "Skinned" : "") << std::endl;

        for (int v = 0; v < attribs.vertCount; v++)
        {
            Vertex vertex;
            
            fseek(fp, bufferBase + (v * attribs.stride), SEEK_SET);
            
            uint16 uv[2];
            fread(uv, sizeof(uint16), 2, fp);
            // TODO: Confirm this divisor
            vertex.tex.x = uv[0] / 2048.f;
            //vertex.tex.y = 1.f - (uv[1] / 2048.f);
            vertex.tex.y = (uv[1] / 2048.f);
            
            uint32 ff;
            fread(&ff, sizeof(uint32), 1, fp);
            ffList.push_back(ff);
            
            if (floatVert)
            {
                float pos[3];
                fread(pos, sizeof(float), 3, fp);
                // TODO: Confirm this divisor
                vertex.pos.x = (pos[0]/30720.f) * modelHeader.modelScale;
                vertex.pos.y = (pos[1]/30720.f) * modelHeader.modelScale;
                vertex.pos.z = (pos[2]/30720.f) * modelHeader.modelScale;
            }
            else
            {
                int16 pos[3];
                fread(pos, sizeof(int16), 3, fp);
                vertex.pos.x = (pos[0]/(float)INT16_MAX) * modelHeader.modelScale;
                vertex.pos.y = (pos[1]/(float)INT16_MAX) * modelHeader.modelScale;
                vertex.pos.z = (pos[2]/(float)INT16_MAX) * modelHeader.modelScale;
            }
            
            if (skinned)
            {
                uint8 weights[4];
                uint8 bones[4];
                fread(weights, sizeof(uint8), 4, fp);
                fread(bones, sizeof(uint8), 4, fp);
                for (int b = 0; b < 4; b++)
                {
                    vertex.weights[b] = weights[b] / (float)INT8_MAX;
                    vertex.bones[b] = attribs.boneMap[bones[b] / 3]; // TODO: Why div 3?
                }
            }
            else
            {
                for (int b = 0; b < 4; b++)
                {
                    vertex.weights[b] = 0;
                    vertex.bones[b] = 0;
                }
            }

            vertices.push_back(vertex);
        }

        MeshSection section = MeshSection();
        section.verts = vertices;
        section.smoothShade = false;
        section.materialName = std::string(texInfo[attribs.texId].name);
        std::cout << section.materialName << std::endl;
        if (attribs.indType == 0x40 || attribs.indType == 0x41)
        {
            for (int v = 0; v < attribs.vertCount - 2; v++)
            {
                if (ffList[v] == 8421504 || ffList[v + 1] == 8421504 || ffList[v + 2] == 8421504)
                    continue;

                section.indices.push_back(v);
                if (v % 2 == 0)
                {
                    section.indices.push_back(v + 2);
                    section.indices.push_back(v + 1);
                }
                else
                {
                    section.indices.push_back(v + 1);
                    section.indices.push_back(v + 2);
                }
            }
        }
        else if (attribs.indType == 0x30 || attribs.indType == 0x32)
        {
            for (int v = 0; v < attribs.vertCount; v += 3)
            {
                section.indices.push_back(v);
                section.indices.push_back(v + 2);
                section.indices.push_back(v + 1);
            }
        }
        else
        {
            std::cout << std::hex;
            std::cout << "UNKOWN GEOM TYPE " << (uint32)attribs.indType << std::endl;
            std::cout << std::dec;
        }

        sections.push_back(section);

        bufferBase += attribs.stride * attribs.vertCount;
    }

    Mesh mesh = Mesh();
    mesh.groups = sections;
    mesh.name = std::string(name.data());
    mesh.materialNames = matNames;
    WriteOBJ(mesh);
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        std::cerr << "NO INPUT" << std::endl;
        PrintHelp();
        return -1;
    }
    FILE * fp = fopen(argv[1], "rb");
    if (fp)
    {
        DumpPMO2(fp);
        fclose(fp);
        return 0;
    }
    else
    {
        std::cerr << "NOT OPEN" << std::endl;
        PrintHelp();
        return -1;
    }
}