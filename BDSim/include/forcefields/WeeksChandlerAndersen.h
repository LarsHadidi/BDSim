#pragma once

#include <math.h>
#include <iostream>
#include <interfaces/ForceField.h>
#include <boost/simd/sdk/simd/pack.hpp>
#include <boost/simd/include/functions/plus.hpp>
#include <boost/simd/include/functions/multiplies.hpp>
#include <boost/simd/include/functions/sum.hpp>
#include <boost/property_tree/ptree.hpp>

namespace forcefields {
	/// \brief Forcefield based on the Weeks-Chandler-Anderson potential.
	/// \author Lars Hadidi
	/// The class implements the ForceField interface.
	class WeeksChandlerAndersen : public interfaces::ForceField {
		public:
			/// \brief Constructor of the forcefield.
			/// \param initConf  Pointer to the initialization object.
			/// \param cellList Pointer to the cell list holding the particles.
			WeeksChandlerAndersen(boost::property_tree::iptree* initConf, datastructures::CellList*const cellList);
			/// \brief Desctructor of the forcefield.
			/// Since there is no dynamically allocated data, nothing has to be freed.
			~WeeksChandlerAndersen();
			/// \brief Calculates the forces for all particles in the linked list passed to the constructor and updating their values.
			void calcForces();
			/// \brief Calculates the virial of the system.
			/// \return The virial.
			double getVirial();
			/// \brief Calculates the total energy of the system.
			/// \return The energy.
			double getEnergy();
		private:
			double const SQUARED_CUTOFF_RADIUS; ///< Sqaured cutoff radius for the short distance interaction.
			double const GRID_DIM_X; ///< Width of the system.
			double const GRID_DIM_Y; ///< Height of the system.
			double virial; ///< Virial of the system.
			double energy; ///< Total energy of the system.
			datastructures::CellList*const CELL_LIST; ///< Pointer to the cell list.
			/// \brief Loops over all cells to calculate the force for each particle in the linked list the cell points into.
			/// \param[in] base_particle The particle with which the interaction will be calculated.
			/// \param[in] node_j The node of the linked list which holds the interaction partner of the base_particle.
			void cellLoop(datastructures::Particle* const base_particle, datastructures::llnode<datastructures::Particle>* node_j);
			/// \brief Calculates the force acting on a pair of particles
			/// \param[in] squaredDistance squared distance between the two particles. Reuse of precomputed value.
			/// \param[in] particle_i First interaction partner
			/// \param[in] particle_j Second interaction partner
			/// \param[in] dX Distance of the two particles in x-dimension in minmum image convention
			/// \param[in] dY Distance of the two particles in y-dimension in minimum image convention
			void calc(const double squaredDistance, datastructures::Particle* const particle_i, datastructures::Particle* const particle_j, const double dX, const double dY);
	};

}

