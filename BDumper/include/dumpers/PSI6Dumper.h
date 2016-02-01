#pragma once

#include <dumpers/Dumper.h>
#include <tools/Options.h>

namespace dumpers {

    class PSI6Dumper : public Dumper {
        #pragma pack(push)
        #pragma pack(1)
        typedef struct {
            double x;
            double y;
            double re;
            double im;
        } PSI6Point;
        #pragma pack(pop)
        public:
            PSI6Dumper(tools::Options& options);
            ~PSI6Dumper();
            void dump();
        private:
            const uint32_t PARTICLES;
            const size_t FRAME_SIZE;
        };
}
