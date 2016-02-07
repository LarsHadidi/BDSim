#include <dumpers/VCELLSDumper.h>

namespace dumpers {
    VCELLSDumper::VCELLSDumper(tools::Options& options) :
    Dumper((options.inputFolder / "VORO-CELLS").string(), (options.outputPath/"VORO-CELLS.dump").string(), options.startFrame, options.endFrame),
    PARTICLES(options.particleCount)
    { }

    VCELLSDumper::~VCELLSDumper() { }

    void VCELLSDumper::dump() {
        this->outputStream << std::setprecision(53.0 * log10(2));
        uint32_t vertexCount;
        int64_t topoCharge;
        double psi6Re;
        double psi6Im;
        Point2D point;

        uint64_t currentFrame = 0;
        while(this->inputStream.peek() != EOF){
            if(currentFrame >= this->START_FRAME && currentFrame <= this->END_FRAME) {
                for (uint32_t i = 0; i < this->PARTICLES; ++i) {
                    this->inputStream.read((char*) &vertexCount, sizeof(uint32_t));
                    this->inputStream.read((char*) &topoCharge, sizeof(int64_t));
                    this->inputStream.read((char*) &psi6Re, sizeof(double));
                    this->inputStream.read((char*) &psi6Im, sizeof(double));

                    if(vertexCount > this->PARTICLES){
                        BOOST_LOG_TRIVIAL(error) << "Vertex count of polygon " << i << " in frame " << currentFrame << " is " << vertexCount
                                    << "\n That value is too high. File layout may be corrupted.";
                        exit(EXIT_FAILURE);
                    }

                    this->outputStream << topoCharge << "," << psi6Re << "," << psi6Im << "\t";
                    for (size_t v = 0; v < vertexCount; ++v) {
                        this->inputStream.read((char*) &point, sizeof(Point2D));
                        this->outputStream << point.x << "," << point.y << ",";
                    }
                    this->outputStream << "\n";
                }
            }
            else {
                for (uint32_t i = 0; i < this->PARTICLES; ++i) {
                    this->inputStream.read((char*) &vertexCount, sizeof(uint32_t));
                    this->inputStream.seekg(sizeof(int64_t) + sizeof(double) + sizeof(double), std::ios::cur);
                    this->inputStream.seekg(vertexCount * sizeof(Point2D), std::ios::cur);
                }
            }
            currentFrame++;
        }
    }
}
