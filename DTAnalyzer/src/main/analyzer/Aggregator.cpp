#include <analyzer/Aggregator.h>

namespace analyzer {
    Aggregator::Aggregator(std::vector<analyzer::Kernel::RMSD_POINTS_MAP*>* ptrResultList) : PTR_RESULT_LIST(ptrResultList) { }

    Aggregator::~Aggregator() { }

    void Aggregator::writeRootMeanSquareDisplacementDistribution(boost::filesystem::path file) {
        std::ofstream outStream(file.string(), std::ios::out);

        size_t t = 0;
        uint64_t tracks = 0;
        tools::Statistics runningStatistics;

        // Iterate as long as there are tracks to average over while increasing the time value
        do {
            // reset track counter and result value for current timepoint
            tracks = 0;
            runningStatistics.clear();

            // Iterate through all results, i.e. the maps associated to the input files
            for(analyzer::Kernel::RMSD_POINTS_MAP* ptrResult : *this->PTR_RESULT_LIST) {

                // Iterate through every track within a result map
                for(analyzer::Kernel::RMSD_POINTS_MAP::iterator it = ptrResult->begin(); it != ptrResult->end(); ++it) {
                    analyzer::DOUBLE_VECTOR* ptrTrackVector = it->second.displacement;

                    // Track exists at the given timepoint
                    if(ptrTrackVector->size() > t) {
                        tracks++;
                        const double SQUARED_DISPLACEMENT = (*ptrTrackVector)[t];
                        runningStatistics.push(SQUARED_DISPLACEMENT);
                    }
                }
            }
            // Write root of mean squared displacement to file, if and only if it exists for the current timepoint
            if(tracks > 0){
                outStream << std::sqrt(runningStatistics.getMean()) << "\t" << (runningStatistics.getStandardDeviation() / std::sqrt(tracks)) << "\n";
            }

            t++;
            // If there are no tracks for the current time steps, the longest living track has been reached, therefore
            // no further tracks will show up
        } while(tracks > 0);
        outStream.flush();
    }

    void Aggregator::writeLifeTimeHistrgram(boost::filesystem::path file) {
        std::ofstream outStream(file.string(), std::ios::out);
        std::map<uint64_t, uint64_t> histogram;

        // Iterate through all results, i.e. the maps associated to the input files
        for(analyzer::Kernel::RMSD_POINTS_MAP* ptrResult : *this->PTR_RESULT_LIST) {
            // Iterate through every track within a result map
            for(analyzer::Kernel::RMSD_POINTS_MAP::iterator it = ptrResult->begin(); it != ptrResult->end(); ++it) {
                size_t lifetime = it->second.displacement->size();
                ++(histogram[lifetime]);
            }
        }

        // Write the histogram to file
        for(std::map<uint64_t, uint64_t>::iterator it = histogram.begin(); it != histogram.end(); ++it) {
            outStream << it->first << "\t" << it->second << "\n";
        }
    }
}