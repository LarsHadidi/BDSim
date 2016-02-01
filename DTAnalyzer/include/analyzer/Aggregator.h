#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <analyzer/Kernel.h>
#include <boost/filesystem/path.hpp>

#include <tools/Statistics.h>

namespace analyzer {
    class Aggregator {
        public:
            Aggregator(std::vector<analyzer::Kernel::RMSD_POINTS_MAP*>* ptrResultList);
            ~Aggregator();
            void writeRootMeanSquareDisplacementDistribution(boost::filesystem::path file);
            void writeLifeTimeHistrgram(boost::filesystem::path file);
        private:
            std::vector<analyzer::Kernel::RMSD_POINTS_MAP*>* const PTR_RESULT_LIST;
    };
}
