#pragma once

#define DIGITS 15 // See Logging.h for derivation

#include <interfaces/Sampler.h>
#include <interfaces/ForceField.h>
#include <datastructures/Datastructures.h>
#include <tools/Logging.h>
#include <boost/property_tree/ptree.hpp>


namespace samplers {
	/// \brief Sampler class to record the system's total energy
	/// \author Lars Hadidi
	/// The total energy is given by the forcefield applied to the system.
	class EnergySampler : public interfaces::Sampler {
		public:
			/// \brief Creates a energy sampler object.
			/// This samplers measures the system's total energy at a given sampling rate. Then it creates a datapoint from a certain number of samples by averaging over them and
			/// writes it to the output. The sampling data is written to an output stream object which flushes its buffer when full to the disk.
			/// \param initConf Pointer to the initialization object.
			/// \param forceField Pointer to the forcefield from which it calculates the overall energy.
			/// \param samplingrate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
			/// \param smpCountPerDatapoint Number of samples to average over when creating a datapoint which will be written to the output.
			EnergySampler(boost::property_tree::iptree* initConf, interfaces::ForceField* const forceField, double samplingrate, double smpCountPerDatapoint);
			/// \brief Destroys the object and flushes the output stream object.
			/// This is important as the buffers may not be completely full after the simulation's termination.
			~EnergySampler();
			/// \brief Measures the system's energy, according to the sampling rate. Must be called every iteration.
			void sample();
			/// \brief Resets the object's internal state. Also removes all content from previously flushed data on disk.
			void reset();
		private:
			interfaces::ForceField* const FORCE_FIELD; ///< Pointer to the forcefield.
			const double SAMPLES_PER_DATAPOINT; ///< Number of samples to average over in order to create a datapoint.
			const double CALLS_PER_SAMPLE; ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
			double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
			double samples; ///< Number of samples taken. Set to zero everytime a datapoint has been dumped.
			std::string output; ///< Path to output file, including filename.
			std::ofstream outStream; ///< Output stream object.
			double cumulativeEnergy; ///< Sum of samples to average over. Set to zero everytime a datapoint has been dumped.
	};
}
