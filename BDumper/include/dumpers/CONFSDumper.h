#pragma once

#include <dumpers/Dumper.h>
#include <tools/Options.h>

namespace dumpers {

    class CONFSDumper : public Dumper {
        #pragma pack(push)
        #pragma pack(1)
        typedef struct {
            double x;
            double y;
            double fX;
            double fY;
        } Particle;
        #pragma pack(pop)
        public:
            CONFSDumper(tools::Options& options);
            ~CONFSDumper();
            void dump();
        private:
            const uint32_t PARTICLES;
            const size_t FRAME_SIZE;
    };
}


