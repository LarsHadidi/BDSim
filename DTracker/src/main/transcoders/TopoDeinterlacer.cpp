#include <transcoders/TopoDeinterlacer.h>

namespace transcoders {

    TopoDeinterlacer::TopoDeinterlacer(boost::filesystem::path& inputFile, boost::filesystem::path& outputPath, const int64_t TOPO_Min, const int64_t TOPO_Max)
    : TOPO_MIN(TOPO_Min), TOPO_MAX(TOPO_Max), DUMMY_FILE((outputPath / std::string("DC_0")).string())
    {
        try {
            this->inStream.open(inputFile.string(), std::ios::in | std::ios::binary);
            const int64_t N = this->TOPO_MAX - this->TOPO_MIN + 1;
            this->outStreamArray = new std::ofstream[N];
            for(int i = 0; i < N; ++i) {
                std::string outputFileName = (outputPath / (std::string("DC_") + boost::lexical_cast<std::string>(i + this->TOPO_MIN))).string();
                this->outStreamArray[i].open(outputFileName, std::ios::out | std::ios::binary);
            }
        }
        catch(std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
            exit(1);
        }
    }

    TopoDeinterlacer::~TopoDeinterlacer() {
        this->inStream.close();
        for(size_t i = 0; i <  this->TOPO_MAX - this->TOPO_MIN + 1; ++i){
            this->outStreamArray[i].close();
        }
        boost::filesystem::remove(DUMMY_FILE);
        delete[] outStreamArray;
    }

    void TopoDeinterlacer::deinterlace() {

        const int64_t N = this->TOPO_MAX - this->TOPO_MIN + 1;
        std::vector<std::vector<geometry::Point2D>> buffers(N);
        for(std::vector<geometry::Point2D> buffer : buffers) {
            buffer.reserve(SZ_POINT * SZ_BUFFER);
        }

        while (this->inStream.peek() != EOF) {
            uint32_t count = 0;
            Defect defect;

            this->inStream.read((char*)&count, sizeof(uint32_t));
            for(uint32_t i = 0; i < count; ++i){
                this->inStream.read((char*)&defect, sizeof(Defect));
                if(defect.topoCharge != 0) {
                    buffers[defect.topoCharge - this->TOPO_MIN].push_back(geometry::Point2D(defect.x, defect.y));
                }
            }

            for(size_t i = 0; i < N; ++i){
                uint32_t sz = buffers[i].size();
                this->outStreamArray[i].write((char*)&sz, sizeof(uint32_t));
                // unsafe(is Point2D aligned ?) but faster
                //this->outStreamArray[i]->write((char*)&(this->buffers[i].front()), sz * sizeof(Point2D));
                // safe
                for(size_t j = 0; j < sz; ++j){
                    this->outStreamArray[i].write((char*)&(buffers[i][j].x()), sizeof(double));
                    this->outStreamArray[i].write((char*)&(buffers[i][j].y()), sizeof(double));
                }
                buffers[i].resize(0);
            }
        }
    }

}
