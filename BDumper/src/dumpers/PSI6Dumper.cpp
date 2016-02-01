#include <dumpers/PSI6Dumper.h>

namespace dumpers {
    PSI6Dumper::PSI6Dumper(tools::Options& options) :
    Dumper((options.inputFolder / "PSI6").string(), options.outputPath.string(), options.startFrame, options.endFrame),
    PARTICLES(options.particleCount),
    FRAME_SIZE(options.particleCount * sizeof(PSI6Point))
    { }

    PSI6Dumper::~PSI6Dumper() { }

    void PSI6Dumper::dump() {
        this->outputStream << std::setprecision(53.0 * log10(2));
        PSI6Point point;
        uint64_t currentFrame = 0;
        while (this->inputStream.peek() != EOF) {
            if(currentFrame >= this->START_FRAME && currentFrame <= this->END_FRAME) {
                for (uint32_t i = 0; i < this->PARTICLES; ++i) {
                    this->inputStream.read((char *) &point, sizeof(PSI6Point));
                    this->outputStream << point.x << "," << point.y << ",";
                    this->outputStream << point.re << "," << point.im << "\n";
                }
            }
            else {
                this->inputStream.seekg(this->PARTICLES * sizeof(PSI6Point), std::ios::cur);
            }
            currentFrame++;
        }
    }
}
