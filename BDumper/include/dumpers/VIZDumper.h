#pragma once

#include <dumpers/Dumper.h>
#include <tools/Options.h>

namespace dumpers {

    class VIZDumper : public Dumper {
        #pragma pack(push)
        #pragma pack(1)
        typedef struct {
            double pX;
            double pY;
            double fX;
            double fY;
            double pXn;
            double pYn;
            uint32_t cID;
        } VisualizationDatapoint;
        #pragma pack(pop)
        public:
            VIZDumper(tools::Options& options);
            ~VIZDumper();
            void dump();
        private:
            const uint32_t PARTICLES;
            const size_t FRAME_SIZE;
            double cellSizeX;
            double cellSizeY;
            uint32_t cellCountX;
            uint32_t cellCountY;
    };

}
