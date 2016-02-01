#pragma once

#include <tools/Logging.h>
#include <datastructures/Datastructures.h>
#include <blackBoxFactories/GridFactory.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <boost/property_tree/ptree.hpp>

namespace blackBoxFactories {
	/// \brief  Class providing factory methods to create various list datastructures used by the simulator
	/// \author Lars Hadidi
	class ListFactory {
		public:
			/// \brief Initialize the Factory.
			/// \param[in] initConf Pointer to the initialization object.
			/// \param[in] grid The initial tesselation from which to start the simulaton.
			static void initialize(boost::property_tree::iptree* initConf,datastructures::Tesselation grid);
			/// \brief Constructs a cell-list holding addresses to its neighbour cells.
			/// \return Pointer to the cell-list.
			static datastructures::CellList* createCellList();
			/// \brief Constructs a singly linked list holding the particles.
			/// \return Pointer to the singly linked list.
			static datastructures::linkedParticleList* createLinkedParticleList();
		private:
			/// \brief Private constructor to enforce sinlgeton usage of the factory
			ListFactory(){}
			static boost::property_tree::iptree* 	initConf; ///< Pointer to the initialization object
			static datastructures::Tesselation  	grid; ///< Initial tesselation data

	};
}
