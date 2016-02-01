#pragma once

#include <interfaces/Interfaces.h>
#include <datastructures/Datastructures.h>
#include <forcefields/ForceFields.h>
#include <boost/property_tree/ptree.hpp>

namespace blackBoxFactories {
    /// \brief  Class providing factory methods to create forcefields.
    /// \author Lars Hadidi
    /// Each factory method return an object which implments the Integrator interface.
    class ForceFieldFactory {
        public:
            /// \brief Initialize the Factory.
            /// \param[in] initConf Pointer to the initialization object.
            /// \param[in] cells Pointer to the cell-list which holds the particles
            static void initialize(boost::property_tree::iptree* initConf,datastructures::CellList*const cells);
            /// \brief Creates a forcefield based on the Weeks-Chandler-Anderson potential
            /// \return Pointer to the forcefield object
            static interfaces::ForceField* createWeeksChandlerAndersonForceField();
        private:
            /// \brief Private constructor to enforce sinlgeton usage of the factory
            ForceFieldFactory(){}

            static boost::property_tree::iptree  *initConf; ///< Pointer to the initialization object
            static datastructures::CellList	 *cells; ///< Pointer to the cell-list holding the particles
    };
}