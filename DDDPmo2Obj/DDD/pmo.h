#pragma once
#include <string>
#include "Utils/BinaryTypes.h"
#include "Utils/BinaryReader.h"
#include "DDD/structs.h"

namespace DreamDropDistance
{
    class PMO_SKELETON;

    class PMO
    {
    public:
        PMO* PmoFromFile(std::string path);
        PMO* ReadPmo(BinaryReader& reader);

    private:
        internal::PMO_FILE_HEADER ReadFileHeader(BinaryReader& reader);
        internal::PMO_MODEL_HEADER ReadModelHeader(BinaryReader& reader);
        internal::PMO_TEX_INFO ReadTexInfo(BinaryReader& reader);
        internal::DDD_GEOM_HEADER ReadGeomHeader(BinaryReader& reader);
        internal::SUB_BUFFER_ATTRIBS ReadSubBufferAttribs(BinaryReader& reader);
    };
}