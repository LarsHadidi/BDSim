#pragma once

#include <interfaces/Interfaces.h>
#include <datastructures/Datastructures.h>
#include <integrators/Integrators.h>
#include <boost/property_tree/ptree.hpp>

namespace blackBoxFactories {
    /// \brief  Class providing factory methods to create integrators
    /// \author Lars Hadidi
    /// Each factory method return an object which implments the ForceField interface
    class IntegratorFactory {
        public:
            /// \brief Initialize the Factory.
            /// \param[in] initConf Pointer to the initialization object.
            /// \param[in] particlesAddress Pointer to address pointing to the linked list holding the particles.
            static void initialize(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress);
            /// \brief Creates an Euler-Maruyama Integrator.
            /// \return Pointer of Integrator-Interface type pointing to the integrator created.
            static interfaces::Integrator* createEulerMaruyamaIntegrator();
        private:
            /// \brief Private constructor to enforce sinlgeton usage of the factory
            IntegratorFactory(){}
            static boost::property_tree::iptree* initConf; ///< Pointer to the initialization object.
            static datastructures::linkedParticleList** ptrAddrParticles; ///< Pointer to address pointing to the linked list holding the particles.
    };
}