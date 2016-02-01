#pragma once

#include <dumpers/Dumper.h>
#include <tools/Options.h>

namespace dumpers {

    class DEFECTSDumper : public Dumper {
        #pragma pack(push)
        #pragma pack(1)
        typedef struct {
            double x;
            double y;
            int64_t topoCharge;
        } Defect;
        #pragma pack(pop)
        public:
            DEFECTSDumper(tools::Options& options);
            ~DEFECTSDumper();
            void dump();
        private:
            const uint32_t PARTICLES;
    };
}