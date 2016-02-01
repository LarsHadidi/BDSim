#include <VoronoiDecomposer/VoronoiDecomposer.h>

namespace VoronoiDecomposer {
    bool VoronoiDecomposer::instantiated = false;
    VoronoiDecomposer::VoronoiDecomposer(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress) :
        DIM_X(initConf->get<double>("dimx")),
        DIM_Y(initConf->get<double>("dimy")),
        PARTICLE_COUNT(initConf->get<uint32_t>("particles")),
        PTR_PTR_PARTICLES(particlesAddress),
        CONTAINER(new  voro::container_2d    (0.0, DIM_X,
                                              0.0, DIM_Y,
                                              (int)ceil(sqrt(((double)PARTICLE_COUNT / (DIM_X * DIM_Y)) / 8) * DIM_X),
                                              (int)ceil(sqrt(((double)PARTICLE_COUNT / (DIM_X * DIM_Y)) / 8) * DIM_Y),
                                              true, true,
                                              16)),
        VORO_CELLS(new std::vector<vCell>(PARTICLE_COUNT))
    {
        if(VoronoiDecomposer::instantiated) throw std::runtime_error("VoronoiDecomposer already instantiated");
        VoronoiDecomposer::instantiated = true;
        this->decomposed = false;
    }
    VoronoiDecomposer::~VoronoiDecomposer(){
        VoronoiDecomposer::instantiated = false;
        delete this->CONTAINER;
        delete this->VORO_CELLS;
    }

    // This method decomposes the domain into voronoi cells for the given particle positions.
    // Due to the fact that on traversal of the voro::container the order of traversal is not known
    // the particle's position corresponding to a cell is stored in a vCell structure. The particles
    // are inserted into the container with ids corresponding to their index in the linkedParticleList.
    // Therefore, the cells' neighbours are those indices, whereas the the id of the cell itself is hidden in the container.
    Decomposition* VoronoiDecomposer::update(){
        if(!this->decomposed) {
            this->CONTAINER->clear();
            for (uint32_t i = 0; i < this->PARTICLE_COUNT; ++i) {
                double x = (*(this->PTR_PTR_PARTICLES))->data[i].value.posX;
                double y = (*(this->PTR_PTR_PARTICLES))->data[i].value.posY;
                assert(x >= 0 && y >= 0 && x <= this->DIM_X && y <= this->DIM_Y);
                this->CONTAINER->put(i, x, y);
            }

            voro::c_loop_all_2d voroParticleIterator(*(this->CONTAINER));

            voroParticleIterator.start();
            uint32_t i = 0;
            do {
                this->CONTAINER->compute_cell(this->VORO_CELLS->at(i).cell, voroParticleIterator);
                this->VORO_CELLS->at(i).x = voroParticleIterator.x();
                this->VORO_CELLS->at(i).y = voroParticleIterator.y();
                ++i;
            } while (voroParticleIterator.inc());
            this->decomposed = true;
        }
        return this->VORO_CELLS;
    }

    void VoronoiDecomposer::finalize(){
        this->decomposed = false;
    }
}