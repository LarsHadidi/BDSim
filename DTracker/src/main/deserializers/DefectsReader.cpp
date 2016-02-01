#include <deserializers/DefectsReader.h>

namespace deserializers {

    DefectsReader::DefectsReader(boost::filesystem::path& file) {
        try {
            this->inStream.open(file.string(), std::ios::in | std::ios::binary);
        }
        catch(std::exception& e){
            BOOST_LOG_TRIVIAL(error) << e.what();
            exit(EXIT_FAILURE);
        }
        if(!inStream){
            BOOST_LOG_TRIVIAL(error) << "Error while opening " << file.string();
            exit(EXIT_FAILURE);
        }
    }

    DefectsReader::~DefectsReader() {
        this->inStream.close();
    }

    bool DefectsReader::hasNext() {
        return !inStream.eof();
    }

    bool DefectsReader::nextFrame(std::vector<geometry::Point2D>* destination) {
        assert(destination->size() == 0);
        if(this->inStream.peek() != EOF){
            uint32_t count;
            Defect defect;

            this->inStream.read((char*)&count, sizeof(uint32_t));

            for(uint32_t i = 0; i < count; ++i){
                this->inStream.read((char*)&defect, sizeof(Defect));
                destination->push_back(geometry::Point2D(defect.x, defect.y));
            }
            return true;
        }
        else {
            return false;
        }
    }

    std::pair<int64_t, int64_t> DefectsReader::findTopoChargeBoundaries() {
        int64_t min = std::numeric_limits<int64_t>::max();
        int64_t max = std::numeric_limits<int64_t>::min();

        this->inStream.clear();
        this->inStream.seekg(std::ios::beg);


        while(this->inStream.good()) {
            uint32_t count = 0;
            Defect defect;

            this->inStream.read((char*)&count, sizeof(uint32_t));
            for(uint32_t i = 0; i < count; ++i){
                this->inStream.read((char*)&defect, sizeof(Defect));
                if (defect.topoCharge > max) max = defect.topoCharge;
                if (defect.topoCharge < min) min = defect.topoCharge;
            }
        }

        this->inStream.clear();
        this->inStream.seekg(std::ios::beg);

        return std::pair<int64_t, int64_t>(min, max);
    }



}
