#pragma once

#define TOPO_CHARGE_BASE 6
#define DIGITS 15 // See Logging.h for derivation

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
    /// \brief Sampler class to record the system's defects configuration
    /// \author Lars Hadidi
    /// Configuration is the set of all defects' positions at a given timepoint.
     class DefectsSampler : public interfaces::Sampler {
        #pragma pack(push)
        #pragma pack(1)
                /// \brief Dataelement which will be written to disk.
                /// Padding has been removed by preprocessor directives to ensure a compressed layout of the binary data.
                typedef struct {
                    double x;
                    double y;
                    int64_t topoCharge;
                } Defect;
        #pragma pack(pop)
        public:
            /// \brief Creates a defects sampler object.
            /// The sampling data is stored into an internal buffer which will be flushed when full to the output stream which itself has its own buffer which is flushed to the operating system's disk management which writes th e data to disk.
            /// \param initConf Pointer to the initialization object.
            /// \param particleAddress Pointer to address pointing to the linked list holding the particles.
            /// \param voronoiDecomposer Pointer to the voronoi decomposer object.
            /// \param samplingrate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
            /// \param szBuffer Number of samples to be held in the internal buffer. As soon as the buffer is full, its contents will be flushed to disk. The output folder is given in the initialization object. The filename is predefined by the class.
            DefectsSampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, uint32_t szBuffer);
            /// \brief Destroys the object and flushes its internal buffer as well as the output stream's buffer.
            /// This is important as the buffers may not be completely full after the simulation's termination.
            ~DefectsSampler();
            /// \brief Records all defects' positions into the internal buffer, according to the sampling rate. Must be called every iteration.
            void sample();
            /// \brief Clears the internal buffer and resets the object's internal state. Also removes all content from previously flushed data on disk.
            void reset();
        private:
            datastructures::linkedParticleList**const PTR_PTR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
            VoronoiDecomposer::VoronoiDecomposer*const VORONOI_DECOMPOSER; ///< Pointer to the voronoi decomposer object.
            const double CALLS_PER_SAMPLE; ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
            double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
            boost::filesystem::path outputPath; ///< Path to output.
            std::ofstream outStream; ///< Output stream object.
            const uint32_t BUFFER_SIZE; ///< Size of the internal buffer.
            Defect** buffer; ///< Internal buffer layouted as a two dimensional array, linearized.
            uint32_t* bufferedDataSize; ///< Number of defects in the current sample (if sample taken)
            uint32_t bufferPosition; ///< Current position within the buffer, i.e. the number of frames recoreded.
            const uint32_t PARTICLE_COUNT; ///< Number of particles in the system.
            /// \brief Flushes the buffer's contents to disk.
            /// Writes all buffer contents to disk. Does not clear or deallocate memory for performance reasons. The value of bufferPosition won't be changed  either.
            void writeBufferUntilPos();
        };
}
