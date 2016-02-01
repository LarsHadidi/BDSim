#pragma once

#include <datastructures/Datastructures.h>
#include <interfaces/Interfaces.h>
#include <iostream>
#include <fstream>

namespace interfaces {
	/// \brief Abstract base class for the sampler.
	/// \author Lars Hadidi
	/// Defines a set of public pure virtual functions to define the interface for all sampler implementations.
	class Sampler {
		public:
			/// \brief Measures a system's quantity and writes it to an output.
			virtual void sample()	=0;
			/// \brief Resets the internal state of the sampler and deletes all contents from output.
			virtual void reset()	=0;
			virtual ~Sampler(){};
	};
}
