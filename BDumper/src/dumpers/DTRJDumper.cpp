#include <dumpers/DTRJDumper.h>

namespace dumpers {
    DTRJDumper::DTRJDumper(tools::Options& options) :
    Dumper("", "", options.startFrame, options.endFrame),
    PARTICLES(options.particleCount)
    {
        inputStream.close();
        outputStream.close();
        const std::regex fileNamePattern("^TR_(-?[0-9]+)$");
        std::smatch matches;

        for(boost::filesystem::directory_entry& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(options.inputFolder), {})) {
            const std::string filename = entry.path().stem().string();
            if (std::regex_search(filename, matches, fileNamePattern)) {
                BOOST_LOG_TRIVIAL(info) << "Found file containing matchings for topological charge " << matches[1];
                chargeMap.push_back(boost::lexical_cast<int>(matches[1]));

                std::ifstream* fIn = new std::ifstream(entry.path().string(), std::ios::in | std::ios::binary);
                std::ofstream* fOut = new std::ofstream((options.outputPath / filename).string(), std::ios::out);

                if(fIn->is_open() == false){
                    BOOST_LOG_TRIVIAL(error) << "Could not open " << entry.path().string() << " for reading";
                    exit(EXIT_FAILURE);
                }
                if(fOut->is_open() == false){
                    BOOST_LOG_TRIVIAL(error) << "Could not open " << (options.outputPath / filename).string() << "for writing";
                    exit(EXIT_FAILURE);
                }

                inputStreams.push_back(fIn);
                outputStreams.push_back(fOut);

            }
        }
    }

    DTRJDumper::~DTRJDumper() {
        for(std::ifstream* stream : this->inputStreams){
            stream->close();
            delete stream;
        }
        for(std::ofstream* stream : this->outputStreams){
            stream->close();
            delete stream;
        }
        chargeMap.clear();
    }

    void DTRJDumper::dump() {
        TrackPoint point;
        uint32_t trackPointCount;

        BOOST_LOG_TRIVIAL(info) << "DTRJ Type will be dumped completely";
        for (size_t c = 0; c < this->inputStreams.size(); ++c) {
            *(this->outputStreams[c]) << std::setprecision(53.0 * log10(2));
            uint64_t currentFrame = 0;
            while (this->inputStreams[c]->peek() != EOF) {
                this->inputStreams[c]->read((char *) &trackPointCount, sizeof(uint32_t));

                if(trackPointCount > this->PARTICLES){
                    BOOST_LOG_TRIVIAL(error) << "Track count in file for charge " << this->chargeMap[c] << " in frame " << currentFrame << " is " << trackPointCount
                                             << "\n That value is too high. File layout may be corrupted.";
                    exit(EXIT_FAILURE);
                }


                for (uint32_t i = 0; i < trackPointCount; ++i) {
                    this->inputStreams[c]->read((char *) &point, sizeof(TrackPoint));

                    *(this->outputStreams[c]) << point.trackID << "," << point.x << "," << point.y << "\t";
                }
                *(this->outputStreams[c]) << "\n";

                currentFrame++;
            }
        }
    }
}
