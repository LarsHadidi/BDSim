#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include <datastructures/Datastructures.h>
#include <buffering/Buffer.h>

#include <boost/filesystem.hpp>

namespace deserializers {
    class DTRJReader {
        public:
            DTRJReader(boost::filesystem::path& file);
            ~DTRJReader();
            const std::vector<std::streampos>* const findSplitPoints();
            bool findMaxTrackID(buffering::Buffer* const BUFFER, uint64_t* id);
            bool findMinTrackID(buffering::Buffer* const BUFFER, uint64_t* id);
        private:
            const std::string STR_FILE;
            std::ifstream* inputStream;
    };
}
