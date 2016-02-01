#pragma once

#include <interfaces/Interfaces.h>
#include <datastructures/Datastructures.h>
#include <tools/Logging.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <boost/property_tree/ptree.hpp>

namespace samplers {
	/// \brief Sampler class to record the system's configuration
	/// \author Lars Hadidi
	/// Configuration is the set of all particles' positions at a given timepoint.
	class ConfigurationsSampler : public interfaces::Sampler {
		public:
			/// \brief Creates a configurations sampler object.
			/// The sampling data is stored into an internal buffer which will be flushed when full to the output stream which itself has its own buffer which is flushed to the operating system's disk management which writes th e data to disk.
			/// \param initConf Pointer to the initialization object.
			/// \param particleAddress Pointer to address pointing to the linked list holding the particles.
			/// \param forceField Pointer to the forcefield object.
			/// \param rate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
			/// \param szBuffer Number of samples to be held in the internal buffer. As soon as the buffer is full, its contents will be flushed to disk. The output folder is given in the initialization object. The filename is predefined by the class.
			ConfigurationsSampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, interfaces::ForceField* const forceField, double samplingrate, uint32_t szBuffer);
			/// \brief Destroys the object and flushes its internal buffer as well as the output stream's buffer.
			/// This is important as the buffers may not be completely full after the simulation's termination.
			~ConfigurationsSampler();
			/// \brief Records all particles' positions into the internal buffer, according to the sampling rate. Must be called every iteration.
			void sample();
			/// \brief Clears the internal buffer and resets the object's internal state. Also removes all content from previously flushed data on disk.
			void reset();
		private:
			datastructures::linkedParticleList**const PTR_PTR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
			interfaces::ForceField* const FORCE_FIELD; ///< Pointer to the forcefield object
			const double CALLS_PER_SAMPLE; ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
			double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
			std::string output; ///< Path to output file, including filename.
			std::ofstream outStream; ///< Output stream object.
			const uint32_t BUFFER_SIZE; ///< Size of the internal buffer.
			datastructures::Particle** buffer; ///< Internal buffer layouted as a two dimensional array, linearized.
			uint32_t bufferPosition; ///< Current position in buffer, which tells how many frames has been written into the buffer.
			const uint32_t PARTICLE_COUNT; ///< Number of particles in the system.
			/// \brief Flushes the buffer's contents to disk.
			/// Writes all buffer contents to disk. Does not clear or deallocate memory for performance reasons. The value of bufferPosition won't be changed  either.
			void writeBufferUntilPos();
	};
};