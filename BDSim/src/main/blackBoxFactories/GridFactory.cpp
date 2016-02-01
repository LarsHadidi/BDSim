#include <blackBoxFactories/GridFactory.h>

namespace blackBoxFactories {
    boost::property_tree::iptree* GridFactory::initConf = nullptr;

    void GridFactory::initialize(boost::property_tree::iptree* initConf) {
        GridFactory::initConf = initConf;
    }

    datastructures::Tesselation GridFactory::createQuadGrid() {

        // Create a rectangular tesselation

        double dimX = GridFactory::initConf->get<double>("dimx");
        double dimY = GridFactory::initConf->get<double>("dimy");
        uint32_t N  = GridFactory::initConf->get<uint32_t>("particles");

        uint32_t slices = std::ceil(std::sqrt(N));
        double cellSizeX = dimX / slices;
        double cellSizeY = dimY / slices;

        const uint64_t POINTS = slices * slices;
        const uint32_t SPILL = POINTS - N;

        datastructures::Tesselation tesselation(POINTS);
        for(uint32_t y = 0; y < slices; ++y){
            for (uint32_t x = 0; x < slices; ++x) {
                tesselation[y * slices + x] = boost::geometry::model::d2::point_xy<double>(x * cellSizeX, y * cellSizeY);
            }
        }

        BOOST_LOG_TRIVIAL(info)<< "\tUsing " << slices << "x" << slices << " tesselation points.";
        BOOST_LOG_TRIVIAL(info)<< "\tSpacing is " << cellSizeX << " in x dimension and " << cellSizeY << " in y dimension.";

        // Remove excess points
        if (initConf->get<std::string>("initstate.tesselation-parameters.particle-distribution").compare("UNIFORM") == 0) {
            GridFactory::uniformRemoveSpill(tesselation, SPILL);
        }
        else if (initConf->get<std::string>("initstate.tesselation-parameters.particle-distribution").compare("SCATTERED") == 0) {
            GridFactory::scatteredRemoveSpill(tesselation, SPILL);
        }
        assert(tesselation.size() == N);
        return tesselation;
    }

    datastructures::Tesselation GridFactory::createHexaGrid() {
        uint32_t N  = GridFactory::initConf->get<uint32_t>("particles");
        const double HEXA_HALF_HEIGHT = std::sqrt(3.0/4.0);
        const uint32_t ROWS = initConf->get<double>("initstate.tesselation-parameters.rows");
        const uint32_t VERTICES_PER_ROW = initConf->get<double>("initstate.tesselation-parameters.row-sites");
        const uint32_t HOLES = initConf->get<uint32_t>("initstate.tesselation-parameters.holes");
        const double DENSITY = initConf->get<double>("initstate.tesselation-parameters.density");

        const double HEXA_EDGE_LENGTH = std::sqrt(1 / ( HEXA_HALF_HEIGHT * DENSITY));


        BOOST_LOG_TRIVIAL(info) << "\tHexagon edge length: " << HEXA_EDGE_LENGTH;
        BOOST_LOG_TRIVIAL(info)<< "\tUsing " << ROWS << "x" << VERTICES_PER_ROW << " tesselation points.";

        datastructures::Tesselation tesselation(ROWS * VERTICES_PER_ROW);
        bool shifted = false;
        for(uint32_t y = 0; y < ROWS; ++y) {
            for (uint32_t x = 0; x < VERTICES_PER_ROW; ++x) {
                tesselation[y * VERTICES_PER_ROW + x] = boost::geometry::model::d2::point_xy<double>(HEXA_EDGE_LENGTH * (x + shifted * 0.5), y * HEXA_HALF_HEIGHT * HEXA_EDGE_LENGTH);
            }
            shifted = !shifted;
        }

        // create defects
        if (HOLES > 0) {
            GridFactory::scatteredRemoveSpill(tesselation, HOLES);
        }
        assert(tesselation.size() == N);

        // calculate grid dimensions
        double dimX = VERTICES_PER_ROW * HEXA_EDGE_LENGTH;
        double dimY = ROWS * (HEXA_EDGE_LENGTH * HEXA_HALF_HEIGHT);

        // add data to initConf
        GridFactory::initConf->put("dimx", dimX);
        GridFactory::initConf->put("dimy", dimY);

        return tesselation;
    }

    void GridFactory::uniformRemoveSpill(datastructures::Tesselation& tesselation,const uint32_t spill) {
            tesselation.erase(tesselation.end() - spill, tesselation.end());
    }

    void GridFactory::scatteredRemoveSpill(datastructures::Tesselation& tesselation,const uint32_t spill) {
        // Init PRNG for array indices to be removed
        PRNGEngineFactory::MTEngine prngEngine = PRNGEngineFactory::createMersenneTwister();
        const int upperBound = GridFactory:: initConf->get<uint32_t>("particles") + spill - 1;
        // Remove elements at random indices
        for(uint32_t i = 0; i < spill; ++i){
            PRNGEngineFactory::UniformIntDistr intDistr(0, upperBound - i);
            tesselation.erase(tesselation.begin() + intDistr(prngEngine));
        }
    }
}