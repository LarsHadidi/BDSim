#pragma once

#include <unordered_map>

#include <datastructures/Datastructures.h>
#include <buffering/Buffer.h>
#include <tools/Options.h>

#include <stxxl.h>

namespace analyzer {
#ifdef SSTXXL
typedef stxxl::VECTOR_GENERATOR<double, 4, 32>::result DOUBLE_VECTOR;
#else
typedef std::vector<double> DOUBLE_VECTOR;
#endif
    typedef struct RMSDPoint_s {
        double initialX;
        double initialY;
        double x;
        double y;
        DOUBLE_VECTOR* displacement;
        RMSDPoint_s()
        {
            #ifdef STXXL
            // WARNING !!! This is a nested critical region which may lead to a deadlock
            // The lock is needed as stxxl vectors can not be initialized in parallel
            #pragma omp critical(RMSDPoint)
            {
                displacement = new DOUBLE_VECTOR();
            }
            #else
            displacement = new DOUBLE_VECTOR();
            #endif
        }
        ~RMSDPoint_s()
        {
            delete displacement;
        }
    } RMSDPoint;

    class Kernel {
        public:
            typedef std::unordered_map<uint64_t, RMSDPoint> RMSD_POINTS_MAP;
            Kernel(buffering::Buffer* buffer);
            ~Kernel();

            RMSD_POINTS_MAP* getDistributionMap();
            void execute();

            static void setup(tools::Options* options);
            static RMSD_POINTS_MAP* aggregate(std::vector<Kernel*>* ptrKernelList);

        private:
            static double squaredDistanceUnderPBC(double x1, double y1, double x0, double y0);
            inline void processTrackPoint(datastructures::TrackPoint* ptrTrackPoint);
            RMSD_POINTS_MAP* distributionMap;
            static RMSD_POINTS_MAP* ptrAggregatedMap;
            static double DIM_X;
            static double DIM_Y;
            static double SIM_TIMESTEPS; // for debugging
            buffering::Buffer* const BUFFER;
    };

}
