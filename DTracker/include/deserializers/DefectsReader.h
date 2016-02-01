#pragma once

#include <boost/filesystem.hpp>
#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>

#include <geometry/Types.h>
#include <tools/Logging.h>


namespace deserializers {
#pragma pack(push)
#pragma pack(1)
    typedef struct {
        double x;
        double y;
        int64_t topoCharge;
    } Defect;
#pragma pack(pop)

    class DefectsReader {
        public:
            DefectsReader(boost::filesystem::path& file);
            ~DefectsReader();

            bool hasNext();
            bool nextFrame(std::vector<geometry::Point2D>* destination);
            std::pair<int64_t, int64_t> findTopoChargeBoundaries();

        private:
            std::ifstream inStream;
    };
}