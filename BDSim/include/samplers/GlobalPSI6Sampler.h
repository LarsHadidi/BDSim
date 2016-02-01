#pragma once

#define DIGITS 15

#include <interfaces/Sampler.h>
#include <VoronoiDecomposer/VoronoiDecomposer.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/path.hpp>
#include <tools/Logging.h>

namespace samplers {
    /// \brief Sampler class to record the system's global bond orientational order parameter.
    /// \author Lars Hadidi
    /// The global bond orientational order parameter is defined as the absolute value of the normed sum over the local bond orientational parameters for every particle.
    class GlobalPSI6Sampler : public interfaces::Sampler {
    public:
        /// \brief Creates a global bond orientational order parameter sampler object.
        /// This samplers measures the system's global bond orientational order parameter at a given sampling rate. Then it creates a datapoint from a certain number of samples by averaging over them and
        /// writes it to the output. The sampling data is written to an output stream object which flushes its buffer when full to the disk.
        /// \param initConf Pointer to the initialization object.
        /// \param particleAddress Pointer to address pointing to the linked list holding the particles.
        /// \param voronoiDecomposer Pointer to the voronoi decomposer object.
        /// \param samplingrate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
        /// \param smpCountPerDatapoint Number of samples to average over when creating a datapoint which will be written to the output.
        GlobalPSI6Sampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, double  smpCountPerDatapoint);
        /// \brief Destroys the object and flushes the output stream object.
        /// This is important as the buffers may not be completely full after the simulation's termination.
        ~GlobalPSI6Sampler();
        /// \brief Measures the system's global bond orientational order parameter, according to the sampling rate. Must be called every iteration.
        void sample();
        /// \brief Resets the object's internal state. Also removes all content from previously flushed data on disk.
        void reset();
    private:
        datastructures::linkedParticleList**const PTR_PTR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
        VoronoiDecomposer::VoronoiDecomposer*const VORONOI_DECOMPOSER; ///< Pointer to the voronoi decomposer object.
        const double CALLS_PER_SAMPLE; ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
        const double SAMPLES_PER_DATAPOINT; ///< Number of samples to average over in order to create a datapoint.
        const uint32_t PARTICLE_COUNT; ///< Number of particles in the system
        double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
        double samples; ///< Number of samples taken. Set to zero everytime a datapoint has been dumped.
        std::ofstream outStream; ///< Output stream object.
        boost::filesystem::path outputPath; ///< Path to output.
        double globalRE; ///< Real part of PSI6
        double globalIM; ///< Imaginary part of PSI6
        double cumulativeAbsoluteValue; /// ///< Sum of samples to average over. Set to zero everytime a datapoint has been dumped.
        std::vector<int> neighbourIDs; ///< Pre-allocated vector to hold IDs of the neighbours for a given particle.
        double DIM_X; ///< Width of the system.
        double DIM_Y; ///< Height of the system.
    };


}
