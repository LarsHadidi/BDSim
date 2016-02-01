#pragma once

#define MATCHER_BUFFER_SZ 16

#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <algorithm>

#include <geometry/Types.h>
#include <tools/Logging.h>
#include <tools/Options.h>
#include <deserializers/PositionsReader.h>

#include <igraph/igraph.h>

class Matcher {
    public:
        Matcher(boost::filesystem::path& inputFile, std::ofstream* outputStream, tools::Options& options);
        ~Matcher();

        void match();
private:
    const double GRID_DIM_X;
    const double GRID_DIM_Y;
    const double WEIGHTS_OFFSET;
    const double THRESHOLD;
    const uint32_t PARTICLE_COUNT;

    std::vector<geometry::Point2D>* previousCoordinates;
    std::vector<geometry::Point2D>* currentCoordinates;
    std::vector<uint64_t>* previousTracks;
    std::vector<uint64_t>* currentTracks;

    std::ofstream* outStream;
    deserializers::PositionsReader* positionsReader;
};
