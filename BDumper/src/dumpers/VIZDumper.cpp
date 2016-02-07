#include <dumpers/VIZDumper.h>

namespace dumpers {
    VIZDumper::VIZDumper(tools::Options& options) :
    Dumper((options.inputFolder / "VIZ").string(), (options.outputPath/"VIZ.dump").string(), options.startFrame, options.endFrame),
    PARTICLES(options.particleCount),
    FRAME_SIZE(options.particleCount * sizeof(VisualizationDatapoint))
    {
        this->inputStream.read((char*)&(this->cellSizeX), sizeof(double));
        this->inputStream.read((char*)&(this->cellSizeY), sizeof(double));
        this->inputStream.read((char*)&(this->cellCountX), sizeof(uint32_t));
        this->inputStream.read((char*)&(this->cellCountY), sizeof(uint32_t));
    }

    VIZDumper::~VIZDumper() { }

    void VIZDumper::dump() {
        this->outputStream << "[" << this->cellSizeX << "," << this->cellSizeY << "," << this->cellCountX << "," << this->cellCountY << "]" << "\n";
        this->outputStream << std::setprecision(53.0 * log10(2));
        VisualizationDatapoint datapoint;

        uint64_t currentFrame = 0;
        while(this->inputStream.peek() != EOF){
            if(currentFrame >= this->START_FRAME && currentFrame <= this->END_FRAME) {
                for (uint32_t i = 0; i < this->PARTICLES; ++i) {
                    this->inputStream.read((char *) &datapoint, sizeof(VisualizationDatapoint));
                    this->outputStream << datapoint.pX << "," << datapoint.pY << ",";
                    this->outputStream << datapoint.pXn << "," << datapoint.pYn << ",";
                    this->outputStream << datapoint.fX << "," << datapoint.fY << ",";
                    this->outputStream << datapoint.cID << "\n";
                }
            }
            else {
                this->inputStream.seekg(this->PARTICLES * sizeof(VisualizationDatapoint), std::ios::cur);
            }
            currentFrame++;
        }
    }
}
