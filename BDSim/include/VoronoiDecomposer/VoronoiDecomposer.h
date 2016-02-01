#pragma once

#include <datastructures/Datastructures.h>
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <container_2d.hh>
#include <c_loops_2d.hh>
#include <cell_2d.hh>
#include <vector>

/// \file VoronoiDecomposer
/// \brief Contains the Decomposer Class and associated datastructures.

namespace VoronoiDecomposer {
    /// \brief Struct connecting a voronoi cell to its corresponding particle.
    typedef struct {
        double x;
        double y;
        voro::voronoicell_neighbor_2d cell;
    } vCell;

    /// \brief Vector of vCells which create the voronoi tesselation.
    typedef std::vector<vCell> Decomposition;

    /// \brief Calss to perform the voronoi decompositon of the domain based on the particles' positions.
    /// \author Lars Hadidi
    /// The class employs a mechanism to keep track if it already decomposed the domain for a given iteration in order
    /// to serve multiple samplers without unneccessary recalculations.
    class VoronoiDecomposer {
        private:
            static bool instantiated; ///< Now let's use a falg to enforce singleton usage and keep the constructor, because I learned about the RAII concept.
            bool decomposed; ///< Flag indicating of a decomposition has been performed.
            datastructures::linkedParticleList**const PTR_PTR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
            const double DIM_X; ///< Width of the system.
            const double DIM_Y; ///< Height of the system.
            const uint32_t PARTICLE_COUNT; ///< Number of particles in the system.
            Decomposition* const VORO_CELLS; ///< The voronoi tesselation of the current frame.
        public:
            voro::container_2d* const CONTAINER; ///< Class for representing a particle system in a two-dimensional rectangular box.
            /// \brief The class constructor sets up the geometry of container and the objects internal state.
            /// \param initConf Pointer to the initialization object.
            /// \param particlesAddress Pointer to address pointing to the linked list holding the particles.
            VoronoiDecomposer(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress);
            /// \brief The class destructor frees the dynamically allocated memory.
            ~VoronoiDecomposer();
            /// \brief This method decomposes the domain into voronoi cells for the given particle positions if finalized.
            /// \return The voronoi decomposition of the system.
            Decomposition* update();
            /// \brief Finalized the decomposer.
            /// Must be called after all samplers are done. This tells the voronoi decomposer that a new iteration is going to start.
            void finalize();
    };
}
