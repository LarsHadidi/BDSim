#pragma once

#include <interfaces/Interfaces.h>
#include <datastructures/Datastructures.h>
#include <samplers/Samplers.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include <tools/Logging.h>
#include <VoronoiDecomposer/VoronoiDecomposer.h>

namespace blackBoxFactories {
	class SamplerFactory {
		/// \brief Class providing factory methods to create samplers.
		/// \author Lars Hadidi
		/// Each factory method return an object which implments the Sampler interface.
		public:
			/// \brief Initialize the Factory.
			/// \param[in] initConf Pointer to the initialization object.
			/// \param[in] particlesAddress Pointer to address pointing to the linked list holding the particles.
			/// \param[in] cells Pointer to the cell-list which holds the particles
			/// \param[in] forceField Pointer to the forcefield used in the simulation
			/// \param[in] voronoiDecomposer Pointer to the VoronoiDecomposer objbect
			static void initialize(boost::property_tree::iptree *initConf, datastructures::linkedParticleList **const particlesAddress,	datastructures::CellList *const cells,	interfaces::ForceField *const forceField, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer);
			/// \brief Creates a sampler object which measures the system's pressure.
			static interfaces::Sampler* createPressureSampler();
			/// \brief Creates a sampler object which measures the system's overall energy.
			static interfaces::Sampler* createEnergySampler();
			/// \brief Creates a sampler object which saves the system's configuration.
			static interfaces::Sampler* createConfigurationsSampler();
			/// \brief Creates a sampler object for the purposes of visualizing the internal simulation mechanisms.
			static interfaces::Sampler* createVisualizationSampler();
			/// \brief Creates a sampler object which saves the topological defects.
			static interfaces::Sampler* createDefectsSampler();
			/// \brief Creates a sampler object which measures the system's defect density.
			static interfaces::Sampler* createDefectDensitiesSampler();
			/// \brief Creates a sampler object which saves the global bond orientational order parameter for each particle.
			static interfaces::Sampler* createPSI6Sampler();
			/// \brief Creates a sampler object which measures the system's global bond orientational order parameter.
			static interfaces::Sampler* createGlobalPSI6Sampler();
			/// \brief Creates a sampler object which saves the voronoi cells and their topological charge as well as their PSI6 vector.
			static interfaces::Sampler* createVoroCellsSampler();

		private:
			/// \brief Private constructor to enforce sinlgeton usage of the factory.
			SamplerFactory(){}
			static boost::property_tree::iptree 		sensorConf; ///<  Property tree holding the configuration data of sensors.ini
			static boost::property_tree::iptree   		*initConf; ///< Pointer to the property tree holding the system's initial configuration data.
			static datastructures::linkedParticleList  	**particlesAddress; ///< Pointer-Pointer to the singly linked list holding the particles.
			static datastructures::CellList				*cells; ///< Pointer to the cell list.
			static interfaces::ForceField	  			*forceField; ///< Pointer to the forcefield object used in the simulation.
			static VoronoiDecomposer::VoronoiDecomposer *voronoiDecomposer; ///< Pointer to the VoronoiDecomposer object.
	};
}
