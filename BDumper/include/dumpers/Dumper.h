#pragma once

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

namespace dumpers {

    class Dumper {
        public:
            Dumper(std::string inputFile, std::string outputFile, uint64_t startFrame, uint64_t endFrame);
            virtual ~Dumper();
            virtual void dump() =0;

        protected:
            std::ifstream inputStream;
            std::ofstream outputStream;
            const uint64_t START_FRAME;
            const uint64_t END_FRAME;
    };

}

