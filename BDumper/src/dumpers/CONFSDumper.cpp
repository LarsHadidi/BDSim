#include <dumpers/CONFSDumper.h>

namespace dumpers {
    CONFSDumper::CONFSDumper(tools::Options& options) :
    Dumper((options.inputFolder / "CONFS").string(), (options.outputPath/"CONFS.dump").string(), options.startFrame, options.endFrame),
    PARTICLES(options.particleCount),
    FRAME_SIZE(options.particleCount * sizeof(Particle))
    { }

    CONFSDumper::~CONFSDumper() { }

    void CONFSDumper::dump() {
        this->outputStream << std::setprecision(53.0 * log10(2));
        Particle particle;
        uint64_t currentFrame = 0;
        while(this->inputStream.peek() != EOF){
            if(currentFrame >= this->START_FRAME && currentFrame <= this->END_FRAME) {
                for (uint32_t i = 0; i < this->PARTICLES; ++i) {
                    this->inputStream.read((char *) &particle, sizeof(Particle));
                    this->outputStream << particle.x << "," << particle.y << ",";
                    this->outputStream << particle.fX << "," << particle.fY << "\n";
                }
            }
            else {
                this->inputStream.seekg(this->PARTICLES * sizeof(Particle), std::ios::cur);
            }
            currentFrame++;
        }
    }
}
