#pragma once

#define DIGITS 15
#define MAX_POLYGON_VERTICES 32
#define VERTEX_SIZE (2 * sizeof(double))
#define TOPO_CHARGE_BASE 6

#include <stdint.h>
#include <interfaces/Interfaces.h>
#include <datastructures/Datastructures.h>
#include <tools/Logging.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <container_2d.hh>
#include <c_loops_2d.hh>
#include <cell_2d.hh>
#include <boost/property_tree/ptree.hpp>
#include <VoronoiDecomposer/VoronoiDecomposer.h>

namespace samplers {
    /// \brief Sampler class to record the system's voronoi tesselation.
    /// \author Lars Hadidi
    /// Records the voronoi tesselation along with the topological charge and local PSI6 value of each cell.
    class VoroCellsSampler : public interfaces::Sampler {
        public:
            /// \brief Creates a voronoi cells sampler object.
            /// \param initConf Pointer to the initialization object.
            /// \param particleAddress Pointer to address pointing to the linked list holding the particles.
            /// \param voronoiDecomposer Pointer to the voronoi decomposer object.
            /// \param samplingrate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
            /// \param szBuffer Number of samples to be held in the internal buffer. As soon as the buffer is full, its contents will be flushed to disk. The output folder is given in the initialization object. The filename is predefined by the class.
            VoroCellsSampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, uint32_t szBuffer);
            /// \brief Destroys the object and flushes its internal buffer as well as the output stream's buffer.
            /// This is important as the buffers may not be completely full after the simulation's termination.
            ~VoroCellsSampler();
            /// \brief Records the voronoi tesselation into the internal buffer, according to the sampling rate. Must be called every iteration.
            void sample();
            /// \brief Clears the internal buffer and resets the object's internal state. Also removes all content from previously flushed data on disk.
            void reset();
        private:
            datastructures::linkedParticleList**const PTR_PTR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
            VoronoiDecomposer::VoronoiDecomposer*const VORONOI_DECOMPOSER; ///< Pointer to the voronoi decomposer object.
            const double CALLS_PER_SAMPLE; ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
            const uint32_t BUFFER_SIZE; ///< Size of the internal buffer.
            double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
            boost::filesystem::path outputPath; ///< Path to output.
            std::ofstream outStream; ///< Output stream object.
            std::vector<int> neighbourIDs; ///< Pre-allocated vector to hold IDs of the neighbours for a given particle.
            std::vector<double> polygon; ///< Vector containing the vertex vectors in the global coordinate system of a voronoi cell, clockwise order, linear layout which means that the vector's size is a multiple of two.
            double** buffer; ///< Internal buffer layouted as a two dimensional array, linearized.
            uint32_t** vertexCount; ///< Two dimensional, linearized array holding the number of vertices of each polygon in the buffer.
            int64_t** topoCharges; ///< Two dimensional, linearized array holding the topological charge values for each polygon in the buffer.
            double** psi6Re; ///< Two dimensional, linearized array holding the real part of PSI6 for each polygon in the buffer.
            double** psi6Im; ///< Two dimensional, linearized array holding the imaginary part of PSI6 for each polygon in the buffer.
            uint32_t bufferPosition; ///< Current position in  buffer, i.e. number of frames recorded.
            const double DIM_X; ///< Width of the system.
            const double DIM_Y; ///< Height of the system.
            const uint32_t PARTICLE_COUNT; ///< Number of particles in the system.
            /// \brief Flushes the buffer's contents to disk.
            /// Writes all buffer contents to disk. Does not clear or deallocate memory for performance reasons. The value of bufferPosition won't be changed  either.
            void writeBufferUntilPos();
            /// \brief Calculates the local bond orientational order parameter for a given voronoi cell,
            /// \param[in] vCell Reference to the voronoi cell from a voronoi decomposition on which to calculate local PSI6.
            /// \param[out] RE Real part of local PSI6.
            /// \param[out] IM Imaginary part of local PSI6.
            inline void calculatePSI6(double& RE, double& IM, VoronoiDecomposer::vCell& vCell);
    };
}
