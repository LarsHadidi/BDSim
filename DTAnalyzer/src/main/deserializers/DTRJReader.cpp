#include <deserializers/DTRJReader.h>

namespace deserializers {

    DTRJReader::DTRJReader(boost::filesystem::path& file) : STR_FILE(file.string()) {
        #pragma omp critical
        {
            this->inputStream = new std::ifstream(this->STR_FILE, std::ios::in | std::ios::binary);
        }
    }
    DTRJReader::~DTRJReader() {
        #pragma omp critical
        {
            this->inputStream->close();
            delete this->inputStream;
        }
    }

    // Finds start positions of each frame
    const std::vector<std::streampos>* const DTRJReader::findSplitPoints() {
        std::vector<std::streampos> *splitPoints = new std::vector<std::streampos>();
        #pragma omp critical
        {
            uint32_t count;
            while (this->inputStream->peek() != EOF) {
                splitPoints->push_back(this->inputStream->tellg());
                this->inputStream->read((char *) &count, sizeof(uint32_t));
                std::streamoff offset = count * sizeof(datastructures::TrackPoint);
                this->inputStream->seekg(this->inputStream->tellg() + offset);
            }
            splitPoints->pop_back(); // last iteration's tellg is the EOF
            inputStream->clear();
        }
        return splitPoints;
    }

    bool DTRJReader::findMinTrackID(buffering::Buffer* const BUFFER, uint64_t* id) {
        BUFFER->reset();
        uint64_t minID = std::numeric_limits<uint64_t>::max();
        bool isEmpty = true;

        uint64_t bytesRead = 0;
        // Only first non-empty frame needed to find minimum
        while(isEmpty && bytesRead < BUFFER->getBufferSize()){
            uint32_t count = *((uint32_t*)BUFFER->getBytes(sizeof(uint32_t)));
            bytesRead += sizeof(count);
            for(uint32_t i = 0; i < count; ++i) {
                isEmpty = false;
                datastructures::TrackPoint trackPoint = *((datastructures::TrackPoint*)BUFFER->getBytes(sizeof(datastructures::TrackPoint)));
                minID = (trackPoint.trackID < minID) ? trackPoint.trackID : minID;
                bytesRead += sizeof(trackPoint);
            }
        }
        *id = minID;
        BUFFER->reset();
        return !isEmpty;
    }

    bool DTRJReader::findMaxTrackID(buffering::Buffer* const BUFFER, uint64_t* id) {
        BUFFER->reset();
        uint64_t maxID = std::numeric_limits<uint64_t>::min();
        bool isEmpty = true;

        uint64_t bytesRead = 0;
        while(bytesRead < BUFFER->getBufferSize()){
            uint32_t count = *((uint32_t*)BUFFER->getBytes(sizeof(uint32_t)));
            bytesRead += sizeof(count);
            for(uint32_t i = 0; i < count; ++i) {
                isEmpty = false;
                datastructures::TrackPoint trackPoint = *((datastructures::TrackPoint*)BUFFER->getBytes(sizeof(datastructures::TrackPoint)));
                maxID = (trackPoint.trackID > maxID) ? trackPoint.trackID : maxID;
                bytesRead += sizeof(trackPoint);
            }
        }
        *id = maxID;
        BUFFER->reset();
        return !isEmpty;
    }
}
