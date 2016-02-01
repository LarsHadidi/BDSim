#include <dumpers/DEFECTSDumper.h>

namespace dumpers {
    DEFECTSDumper::DEFECTSDumper(tools::Options& options) :
    Dumper((options.inputFolder / "DEFECTS-CONFIGURATIONS").string(), options.outputPath.string(), options.startFrame, options.endFrame),
    PARTICLES(options.particleCount)
    { }

    DEFECTSDumper::~DEFECTSDumper() { }

    void DEFECTSDumper::dump() {
        this->outputStream << std::setprecision(53.0 * log10(2));
        Defect defect;
        uint32_t defectCount;

        uint64_t currentFrame = 0;
        while (this->inputStream.peek() != EOF) {
            if(currentFrame >= this->START_FRAME && currentFrame <= this->END_FRAME) {
                this->inputStream.read((char *) &defectCount, sizeof(uint32_t));

                if(defectCount > this->PARTICLES){
                    BOOST_LOG_TRIVIAL(error) << "Defect count in frame " << currentFrame << " is " << defectCount
                                             << "\n That value is too high. File layout may be corrupted.";
                    exit(EXIT_FAILURE);
                }

                for (uint32_t i = 0; i < defectCount; ++i) {
                    this->inputStream.read((char *) &defect, sizeof(Defect));
                    this->outputStream << defect.x << "," << defect.y << "," << defect.topoCharge << "\t";
                }
                this->outputStream << "\n";
            }
            else {
                this->inputStream.read((char *) &defectCount, sizeof(uint32_t));
                this->inputStream.seekg(defectCount * sizeof(Defect), std::ios::cur);
            }
            currentFrame++;
        }
    }
}
