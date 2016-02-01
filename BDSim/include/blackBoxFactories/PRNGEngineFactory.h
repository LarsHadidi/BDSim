#pragma once

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/seed_seq.hpp>
#include <random>
#include <chrono>
#include <fstream>
#include <tools/Logging.h>



namespace blackBoxFactories {

	class PRNGEngineFactory {
		/// \brief  Class providing methods to create pseudo-random number generators
		/// \author Lars Hadidi
		public:
			typedef boost::mt19937 MTEngine; ///< Mersenne Twister type
			typedef boost::uniform_int<> UniformIntDistr; ///< Uniform target distribution template parameter
			typedef boost::normal_distribution<double> GaussDistr; ///< Gaussian target distribution template parameter using Ziggurat algorithm.
			/// \brief Creates a Mersenne Twister pseudo-random number generator.
			/// The generator is seeded by either 32 bytes from the /dev/urandom device, if available, or
			/// a 8 byte double holding the amount of ticks since the default epoch timepoint with the smallest tick period
			/// provided by the system. Its periodicity is of magnitude 10 to the 6000.
			/// \return Returns the pseudo-random number engine, which can be bound to a target distribution
			static MTEngine createMersenneTwister();
	};

}
