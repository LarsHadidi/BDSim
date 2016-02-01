#pragma once

#include <dumpers/Dumper.h>
#include <tools/Options.h>

namespace dumpers {

    class VCELLSDumper : public Dumper {
        #pragma pack(push)
        #pragma pack(1)
        typedef struct {
            double x;
            double y;
        } Point2D;
        #pragma pack(pop)
    public:
        VCELLSDumper(tools::Options& options);
        ~VCELLSDumper();
        void dump();
    private:
        const uint32_t PARTICLES;
    };

}