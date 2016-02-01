#include <blackBoxFactories/IntegratorFactory.h>

namespace blackBoxFactories {

    boost::property_tree::iptree* IntegratorFactory::initConf = nullptr;
    datastructures::linkedParticleList** IntegratorFactory::ptrAddrParticles = nullptr;

    void IntegratorFactory::initialize(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress) {
        IntegratorFactory::initConf = initConf;
        IntegratorFactory::ptrAddrParticles = particlesAddress;
    }

    interfaces::Integrator* IntegratorFactory::createEulerMaruyamaIntegrator() {
        return new integrators::EulerMaruyama(IntegratorFactory::initConf, IntegratorFactory::ptrAddrParticles);
    }
}