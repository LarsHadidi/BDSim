#pragma once

#include <datastructures/Datastructures.h>

namespace interfaces {
	/// \brief Abstract base class for the integrator.
	/// \author Lars Hadidi
	/// Defines a set of public pure virtual functions to define the interface for all integrator implementations.
	class Integrator {
		public:
			/// \brief Numerically integrates the of the equations of motion.
			virtual void integrate()	=0;
			virtual ~Integrator(){};
	};
}
