#pragma once

#include <datastructures/Datastructures.h>

namespace interfaces {
	/// \brief Abstract base class for the forcefield.
	/// \author Lars Hadidi
	/// Defines a set of public pure virtual functions to define the interface for all forcefield implementations.
	class ForceField {
		public:
			/// \brief Calculates the forces for all particles in place.
			virtual void calcForces()	=0;
			/// \brief Calculates the virial of the system.
			/// \return The virial.
			virtual double getVirial()	=0;
			/// \brief Calculates the total energy of the system.
			/// \return The energy.
			virtual double getEnergy()	=0;
			virtual ~ForceField(){};
	};
}
