#pragma once

#define DIGITS 15
#define TOPO_CHARGE_BASE 6

#include <tools/Logging.h>
#include <interfaces/Sampler.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <VoronoiDecomposer/VoronoiDecomposer.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iomanip>

namespace samplers {
    /// \brief Sampler class to record the system's local bond orientational order parameter.
    /// \author Lars Hadidi
    /// The local bond orientational order parameter is defined per particles as sum over Euler's number to the power of (six times the imaginary unit times the angle between the particle and its neighbour) for each of its neighbours.
    /// It is a complex number which is stores for each particle.
    class PSI6Sampler : public interfaces::Sampler {
        typedef boost::geometry::model::d2::point_xy<double, double> Point2D;
        #pragma pack(push)
        #pragma pack(1)
                typedef struct {
                    double re = 0;
                    double im = 0;
                } OrderParameter;
                /// \brief Dataelement which will be written to disk.
                /// Padding has been removed by preprocessor directives to ensure a compressed layout of the binary data.
                typedef struct {
                    Point2D position = Point2D(0, 0);
                    OrderParameter orderParameter;
                } PSI6Point;
        #pragma pack(pop)
        public:
            /// \brief Creates a local bond orientational order parameter sampler object.
            /// The sampling data is stored into an internal buffer which will be flushed when full to the output stream which itself has its own buffer which is flushed to the operating system's disk management which writes th e data to disk.
            /// \param initConf Pointer to the initialization object.
            /// \param particleAddress Pointer to address pointing to the linked list holding the particles.
            /// \param voronoiDecomposer Pointer to the voronoi decomposer object.
            /// \param samplingrate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
            /// \param szBuffer Number of samples to be held in the internal buffer. As soon as the buffer is full, its contents will be flushed to disk. The output folder is given in the initialization object. The filename is predefined by the class.
            PSI6Sampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, uint32_t szBuffer);
            /// \brief Destroys the object and flushes its internal buffer as well as the output stream's buffer.
            /// This is important as the buffers may not be completely full after the simulation's termination.
            ~PSI6Sampler();
            /// \brief Records all local bond orientational order parameter values into the internal buffer, according to the sampling rate. Must be called every iteration.
            void sample();
            /// \brief Clears the internal buffer and resets the object's internal state. Also removes all content from previously flushed data on disk.
            void reset();
        private:
            datastructures::linkedParticleList**const PTR_PTR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
            VoronoiDecomposer::VoronoiDecomposer*const VORONOI_DECOMPOSER; ///< Pointer to the voronoi decomposer object.
            const double CALLS_PER_SAMPLE;  ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
            double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
            std::ofstream outStream; ///< Output stream object.
            boost::filesystem::path outputPath; ///< Path to output
            std::vector<int> neighbourIDs;///< Pre-allocated vector to hold IDs of the neighbours for a given particle.
            const uint32_t BUFFER_SIZE; ///< Size of the internal buffer.
            PSI6Point** buffer; ///< Internal buffer layouted as a two dimensional array, linearized.
            uint32_t bufferPosition; ///< Current position within the buffer, i.e. the number of frames recoreded.
            const uint32_t PARTICLE_COUNT; ///< Number of particles in the system.
            const double DIM_X; ///< Width of the system..
            const double DIM_Y; ///< Height of the system.
            /// \brief Flushes the buffer's contents to disk.
            /// Writes all buffer contents to disk. Does not clear or deallocate memory for performance reasons. The value of bufferPosition won't be changed  either.
            void writeBufferUntilPos();
    };
}