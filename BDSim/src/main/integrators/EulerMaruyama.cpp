#include <integrators/EulerMaruyama.h>

namespace integrators {


	EulerMaruyama::EulerMaruyama(boost::property_tree::iptree* initConf, datastructures::linkedParticleList** const particles) :
		PRNG(blackBoxFactories::PRNGEngineFactory::createMersenneTwister(), blackBoxFactories::PRNGEngineFactory::GaussDistr(0, 1)),
		// Calling custom constructor only possible in signature, variate_generator has no default constructor
		// Reference members must also be initialized in initializer list
		PTR_ADDR_PARTICLES(particles),
		TIMESTEP(initConf->get<double>("timestep")),
		INVERSE_TEMPERATURE(1 / initConf->get<double>("temperature")),
		SQRT_2TIMESTEP(std::sqrt(2.0 * initConf->get<double>("timestep"))),
		DIM_X(initConf->get<double>("dimx")),
		DIM_Y(initConf->get<double>("dimy")),
		FACTOR(TIMESTEP * INVERSE_TEMPERATURE)
	{

	}

	EulerMaruyama::~EulerMaruyama(){
	}

	void EulerMaruyama::integrate(){
		datastructures::Particle* particle;

		for(uint32_t n = 0; n < (*(this->PTR_ADDR_PARTICLES))->count; ++n){
			particle = &((*(this->PTR_ADDR_PARTICLES))->data[n].value);

			// SIMD-Version of	particle->posX += this->TIMESTEP * this->INVERSE_TEMPERATURE * particle->forceX + this->SQRT_2TIMESTEP * this->PRNG();
			//					particle->posY += this->TIMESTEP * this->INVERSE_TEMPERATURE * particle->forceY + this->SQRT_2TIMESTEP * this->PRNG();


			double PRN_X = this->SQRT_2TIMESTEP * this->PRNG();
			double PRN_Y = this->SQRT_2TIMESTEP * this->PRNG();

			boost::simd::pack<double> XMM_POS(particle->posX, particle->posY);
			boost::simd::pack<double> XMM_PRN(PRN_X, PRN_Y);
			boost::simd::pack<double> XMM_FORCE(particle->forceX, particle->forceY);
			boost::simd::pack<double> XMM_RESULT = FACTOR * XMM_FORCE + XMM_PRN + XMM_POS;

			particle->posX = XMM_RESULT[0];
			particle->posY = XMM_RESULT[1];



			// Periodic boundary conditions
			if(particle->posX > DIM_X) {
				particle->posX -= DIM_X;
			}
			else if (particle->posX < 0) {
				particle->posX += DIM_X;
			}

			if(particle->posY > DIM_Y) {
				particle->posY -= DIM_Y;
			}
			else if (particle->posY < 0) {
				particle->posY += DIM_Y;
			}
		}
	}
}
