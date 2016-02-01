#pragma once

#define SZ_POINT 16
#define SZ_BUFFER 128

#include <boost/filesystem.hpp>
#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>

#include <geometry/Types.h>
#include <tools/Logging.h>

namespace transcoders {
#pragma pack(push)
#pragma pack(1)
    typedef struct {
        double x;
        double y;
        int64_t topoCharge;
    } Defect;
#pragma pack(pop)
    class TopoDeinterlacer {
        public:
            TopoDeinterlacer(boost::filesystem::path& inputFile, boost::filesystem::path& outputPath, const int64_t TOPO_Min, const int64_t TOPO_Max);
            ~TopoDeinterlacer();
            void deinterlace();

        private:
            const std::string DUMMY_FILE;
            std::ifstream inStream;
            std::ofstream* outStreamArray;
            const int64_t TOPO_MIN;
            const int64_t TOPO_MAX;
    };

}

