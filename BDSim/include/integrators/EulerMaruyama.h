#pragma once

#include <datastructures/Datastructures.h>
#include <interfaces/Integrator.h>
#include <forcefields/WeeksChandlerAndersen.h>
#include <blackBoxFactories/PRNGEngineFactory.h>
#include <math.h>
#include <boost/simd/pack.hpp>
#include <boost/simd/function/plus.hpp>
#include <boost/simd/function/multiplies.hpp>
#include <boost/property_tree/ptree.hpp>

namespace integrators {
/// \brief Integrator performing the Euler-Maruyama numerical integration scheme.
/// \author Lars Hadidi
/// The class implements the Integrator-Interface.
	class EulerMaruyama : public interfaces::Integrator {
		public:
			/// \brief Constructor of the Integrator.
			/// \param initConf Pointer to the initialization object.
			/// \param particles Pointer to address pointing to the linked list holding the particles.
			EulerMaruyama(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particles);
			/// \brief Desctructor of the Integrator.
			/// Since there is no dynamically allocated data, nothing has to be freed.
			~EulerMaruyama();
			/// \brief Integrates the of the equations of motion.
			/// Propagates to time evolution of the system by updating the particle's positions. The Euler-Maruyama method is employed to numerically solve the equations of motion.
			void integrate();
		private:
			boost::variate_generator<blackBoxFactories::PRNGEngineFactory::MTEngine, blackBoxFactories::PRNGEngineFactory::GaussDistr> PRNG; ///< Random number generator needed for the integration of the stochstic differential equation.
			datastructures::linkedParticleList** const PTR_ADDR_PARTICLES; ///< Pointer to address pointing to the linked list holding the particles.
			// The following values will be calculated at construction of the object and stored as constant values.
			// This is necessary as the compiler will not be able to infere if those values can be extracted from loops even if they will be declared as constant
			// variables within a loop as the right value of the assignment is a non-terminal coming from another part of the program.
			const double TIMESTEP; ///< Value of the timestep used in the numerical integration.
			const double INVERSE_TEMPERATURE; ///< Value of one over temperatur used in the numerical integration, precalculated constant member.
			const double SQRT_2TIMESTEP; ///< Square root of two times timestep used in the numerical integration, precalculated constant member.
			const double DIM_X; ///< Size of the system in x dimension, constant member.
			const double DIM_Y; ///< Size of the system in y dimension, constant member.
			const double FACTOR; ///< TIMESTEP*INVERSE_TEMPERATUR - value needed for the numerical integration, precalculated constant member.

	};

}
