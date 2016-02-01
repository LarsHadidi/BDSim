#pragma once

#define DIGITS 15 // See Logging.h for derivation

#include <interfaces/Sampler.h>
#include <interfaces/ForceField.h>
#include <datastructures/Datastructures.h>
#include <tools/Logging.h>
#include <iostream>
#include <iomanip>
#include <boost/property_tree/ptree.hpp>


namespace samplers {
	/// \brief Sampler class to record the system's pressure.
	/// \author Lars Hadidi
	/// The pressure is calculated via the virial equation. The forcefield delivers the value of the virial.
	class PressureSampler : public interfaces::Sampler {
		public:
			/// \brief Creates a pressure sampler object.
			/// This samplers measures the system's pressure at a given sampling rate. Then it creates a datapoint from a certain number of samples by averaging over them and
			/// writes it to the output. The sampling data is written to an output stream object which flushes its buffer when full to the disk.
			/// \param initConf Pointer to the initialization object.
			/// \param forceField Pointer to the forcefield from which the virial is obtained to calculate the pressure.
			/// \param samplingrate Normed samplingrate, leading to sample every (1 over rate) timestep. A value of 1 means that every timestep will be sampled, a value of 0 means no sampling.
			/// \param smpCountPerDatapoint Number of samples to average over when creating a datapoint which will be written to the output.
			PressureSampler(boost::property_tree::iptree* initConf, interfaces::ForceField *const forceField, double samplingrate, double smpCountPerDatapoint);
			/// \brief Destroys the object and flushes the output stream object.
			/// This is important as the buffers may not be completely full after the simulation's termination.
			~PressureSampler();
			/// \brief Measures the system's pressure, according to the sampling rate. Must be called every iteration.
			void sample();
			/// \brief Resets the object's internal state. Also removes all content from previously flushed data on disk.
			void reset();
		private:
			interfaces::ForceField* const FORCE_FIELD; ///< Pointer to the forcefield.
			const double A; ///< Area of the domain
			const double SAMPLES_PER_DATAPOINT; ///< Number of samples to average over in order to create a datapoint.
			const double CALLS_PER_SAMPLE; ///< As the samplers is called on every iteration, this value holds the number of calls after which a sample may be taken, derived from the sampling rate of type double.
			const uint32_t PARTICLE_COUNT; ///< Number of particles in the system.
			double smpCalls; ///< Counts the number of calls to the sampler. Must be compared to a double value.
			double samples; ///< Number of samples taken. Set to zero everytime a datapoint has been dumped.
			std::string output; ///< Path to outputfile, including filename.
			std::ofstream outStream; ///< Output stream object.
			const double temperature; ///< System's given temperature.
			double cumulativeVirial; ///< Sum of samples to average over. Set to zero everytime a datapoint has been dumped.

	};
}
