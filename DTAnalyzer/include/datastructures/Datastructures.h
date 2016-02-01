#pragma once

#include <stdint.h>

namespace datastructures {
    #pragma pack(push)
    #pragma pack(1)
        typedef struct {
            uint64_t trackID;
            double x;
            double y;
        } TrackPoint;
    #pragma pack(pop)
}