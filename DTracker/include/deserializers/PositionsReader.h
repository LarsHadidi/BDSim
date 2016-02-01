#pragma once

#include <boost/filesystem.hpp>
#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <string.h>

#include <geometry/Types.h>
#include <tools/Logging.h>
#include <tools/Options.h>
#include <buffering/Buffer.h>

namespace deserializers {
    
    class PositionsReader {
    public:
        PositionsReader(boost::filesystem::path& file, tools::Options& options);
        ~PositionsReader();

        bool hasNext();
        void nextFrame(std::vector<geometry::Point2D>* destination);

    private:
        buffering::Buffer* buffer;
    };
}