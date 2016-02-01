#pragma once

#include <dumpers/Dumper.h>
#include <tools/Options.h>
#include <regex>
#include <vector>

namespace dumpers {
    class DTRJDumper : public Dumper {
        #pragma pack(push)
        #pragma pack(1)
        typedef struct TrackPoint_s {
            uint64_t trackID;
            double x;
            double y;
        } TrackPoint;
        #pragma pack(pop)
        public:
            DTRJDumper(tools::Options& options);
            ~DTRJDumper();
            void dump();
        private:
            std::vector<std::ifstream*> inputStreams;
            std::vector<std::ofstream*> outputStreams;
            std::vector<int> chargeMap;
            const uint32_t PARTICLES;
    };
}
