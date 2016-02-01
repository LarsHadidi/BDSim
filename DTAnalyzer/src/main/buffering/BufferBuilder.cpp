#include <buffering/BufferBuilder.h>

namespace buffering {
    BufferBuilder::BufferBuilder(boost::filesystem::path& file, size_t fileSize, unsigned int bufferCount, size_t cacheSize) :
    STR_FILE(file.string()), FILE_SIZE(fileSize), BUFFER_COUNT(bufferCount), CACHE_SIZE(cacheSize)
    { }

    BufferBuilder::~BufferBuilder() { }

    // Find dispatchpoints form list of splitpoints. First dispatchpoint is the first splitpoint, last dispatchpoint is
    // the beginning for the last buffer which should go until the end of the file
    const std::vector<std::streampos>* const BufferBuilder::dispatchPoints(const std::vector<std::streampos>* const SPLIT_POINTS) const {
        std::vector<std::streampos>*dispatchPoints = new std::vector<std::streampos>();
        dispatchPoints->reserve(BUFFER_COUNT);

        const uint64_t PROPOSAL_STEP = this->FILE_SIZE / this->BUFFER_COUNT;
        std::streampos basePos = 0;

        while(dispatchPoints->size() < BUFFER_COUNT) {
            std::streampos dispatchPoint = 0;
            bool dispatchPointFound = false;
            uint64_t i = 0;
            while (!dispatchPointFound) {
                const std::streampos splitPoint = SPLIT_POINTS->at(i);
                const std::streampos splitPointNeighbourRight = SPLIT_POINTS->at(i + 1);
                if (splitPoint < basePos && splitPointNeighbourRight > basePos) {
                    const std::streamsize distanceLeft = abs(splitPoint - basePos);
                    const std::streamsize distanceRight = abs(splitPointNeighbourRight - basePos);
                    dispatchPoint = (distanceLeft < distanceRight) ? splitPoint : splitPointNeighbourRight;
                    dispatchPointFound = true;
                }
                else if (splitPoint == basePos) {
                    dispatchPoint = basePos;
                    dispatchPointFound = true;
                }
                i++;
                if (!dispatchPointFound && i == SPLIT_POINTS->size() - 1) {
                    uint32_t minDistancePoint = SPLIT_POINTS->at(0);
                    for (std::streampos splitPoint : *SPLIT_POINTS) {
                        const std::streamsize distance = abs(splitPoint - basePos);
                        const std::streamsize minDistance = abs(minDistancePoint - basePos);
                        if (distance < minDistance) minDistancePoint = splitPoint;
                    }
                    dispatchPoint = minDistancePoint;
                    dispatchPointFound = true;
                }
            }
            dispatchPoints->push_back(dispatchPoint);
            basePos += PROPOSAL_STEP;
        }
        assert(dispatchPoints->at(0) == SPLIT_POINTS->at(0));
        for (int j = 0; j < dispatchPoints->size(); ++j) {
            assert(std::find(SPLIT_POINTS->begin(), SPLIT_POINTS->end(), dispatchPoints->at(j)) != SPLIT_POINTS->end());
        }
        return dispatchPoints;
    }

    std::vector<Buffer>* const BufferBuilder::createBuffers(const std::vector<std::streampos>* const DISPATCH_POINTS) const {
        std::vector<Buffer>* buffers = new std::vector<Buffer>();
        buffers->reserve(this->BUFFER_COUNT);
        for(size_t i = 1; i < this->BUFFER_COUNT; ++i) {
            buffers->emplace_back(this->STR_FILE, DISPATCH_POINTS->at(i - 1), DISPATCH_POINTS->at(i) - DISPATCH_POINTS->at(i - 1), this->CACHE_SIZE);
        }
        buffers->emplace_back(this->STR_FILE, DISPATCH_POINTS->back(),this->FILE_SIZE - DISPATCH_POINTS->back(), this->CACHE_SIZE);
        return buffers;
    }

}
