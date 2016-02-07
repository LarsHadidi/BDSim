#include <dumpers/Dumper.h>

namespace dumpers {
    Dumper::Dumper(std::string inputFile, std::string outputPath, uint64_t startFrame, uint64_t endFrame) :
    START_FRAME(startFrame), END_FRAME(endFrame)
    {
        this->inputStream.open(inputFile  , std::ios::in | std::ios::binary);
        this->outputStream.open((boost::filesystem::path(outputPath)/"params").string(), std::ios::out);
    }
    Dumper::~Dumper() {
        this->inputStream.close();
        this->outputStream.close();
    }

}
