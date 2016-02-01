#include <deserializers/PositionsReader.h>
#include <tools/Options.h>

namespace deserializers {

    PositionsReader::PositionsReader(boost::filesystem::path& file, tools::Options& options) {
        try {
            this->buffer = new buffering::Buffer(file.string(), 0, boost::filesystem::file_size(file), options.cacheSize);
        }
        catch(std::exception& e){
            BOOST_LOG_TRIVIAL(error) << e.what();
            exit(EXIT_FAILURE);
        }

    }

    PositionsReader::~PositionsReader() {
        delete this->buffer;
    }

    bool PositionsReader::hasNext() {
        return this->buffer->hasBytes();
    }

    void PositionsReader::nextFrame(std::vector<geometry::Point2D>* destination) {
        assert(destination->size() == 0);

        uint32_t count;
        double x;
        double y;

        count = *((uint32_t*)(this->buffer->getBytes(sizeof(uint32_t))));

        for(uint32_t i = 0; i < count; ++i){
            x = *((double*)(this->buffer->getBytes(sizeof(double))));
            y = *((double*)(this->buffer->getBytes(sizeof(double))));
            destination->push_back(geometry::Point2D(x, y));
        }
    }




}
