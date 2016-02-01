#include <forcefields/WeeksChandlerAndersen.h>

namespace forcefields {

	WeeksChandlerAndersen::WeeksChandlerAndersen(boost::property_tree::iptree* initConf, datastructures::CellList* const cellList) :
		SQUARED_CUTOFF_RADIUS(initConf->get<double>("squaredCutoffRadius")),
		GRID_DIM_X(initConf->get<double>("dimx")),
		GRID_DIM_Y(initConf->get<double>("dimy")),
		CELL_LIST(cellList)
	{
		this->virial = 0;
		this->energy = 0;
	}

	WeeksChandlerAndersen::~WeeksChandlerAndersen() { }


	inline void WeeksChandlerAndersen::cellLoop(datastructures::Particle* const base_particle, datastructures::llnode<datastructures::Particle>* node_j){
		while(node_j){
			datastructures::Particle* const particle_j = &(node_j->value);
			const double dX = particle_j->posX - base_particle->posX;
			const double dY = particle_j->posY - base_particle->posY;

			// Minimum Image Convention revisited
			double dX_NPI = 0;
			double dY_NPI = 0;
			const double dXabs = fabs(dX);
			const double dYabs = fabs(dY);
			// (distance > 2 * cellSize) is the true trigger, but
			// that only happens when we wrap around the grid
			if(dXabs > GRID_DIM_X / 2.0) {
				const double d = GRID_DIM_X - dXabs;
				dX_NPI = (dX > 0) ? - d : + d; // and finally restore the direction relative to the reference particle
			}
			else {
				dX_NPI = dX;
			}
			if (dYabs > GRID_DIM_Y / 2.0) {
				const double d = GRID_DIM_Y - dYabs;
				dY_NPI = (dY > 0) ? - d : + d;
			}
			else {
				dY_NPI = dY;
			}

			const double squaredDistance = dX_NPI * dX_NPI + dY_NPI * dY_NPI;
			if(squaredDistance < WeeksChandlerAndersen::SQUARED_CUTOFF_RADIUS) {
				calc(squaredDistance, base_particle, particle_j, dX_NPI, dY_NPI);
			}
			node_j = node_j->next;
		}
	}

	inline void WeeksChandlerAndersen::calc(const double squaredDistance, datastructures::Particle* const particle_i, datastructures::Particle* const particle_j, const double dX, const double dY){
			// Calculate necessary expressions
			const double oneOverSquaredDistance = 1 / squaredDistance;
			const double oneOverDistanceToTheSix = oneOverSquaredDistance * oneOverSquaredDistance * oneOverSquaredDistance;

			// Calculate scalar forcefield factor
			const double forceFieldValue = 48.0 * oneOverSquaredDistance * oneOverDistanceToTheSix * (oneOverDistanceToTheSix - 0.5);

			// Forces due to potential
			boost::simd::pack<double> XMM_POS(dX, dY);
			boost::simd::pack<double> XMM_FORCE = forceFieldValue * XMM_POS;


			//double fX = forceFieldValue * dX;
			//double fY = forceFieldValue * dY;
			// Considering every j in the potential of i, the force acts on j
			particle_j->forceX += XMM_FORCE[0];
			particle_j->forceY += XMM_FORCE[1];

			// Looping through non commuting pairs leads to the necessity of using Newton's third law
			particle_i->forceX -= XMM_FORCE[0];
			particle_i->forceY -= XMM_FORCE[1];

			// Calculate the Clausius Virial with the help of Newton's third law
			this->virial += boost::simd::sum(XMM_FORCE * XMM_POS);
			// Calculate the energy
			this->energy += 4 * oneOverDistanceToTheSix * (oneOverDistanceToTheSix - 1) + 1;

	}

	void WeeksChandlerAndersen::calcForces(){

		// *** Reset values **************************
		this->virial = 0;
		this->energy = 0;

		for(uint64_t i = 0; i < this->CELL_LIST->cellCountTotal; ++i){

			datastructures::Cell* cell = this->CELL_LIST->data + i;
			datastructures::llnode<datastructures::Particle>* node_i = cell->pListHead;

			while(node_i){
				datastructures::Particle* particle_i = &(node_i->value);
				datastructures::llnode<datastructures::Particle>* node_j;

				node_j = node_i->next;
				cellLoop(particle_i, node_j);



				node_j = cell->NEIGHBOURS.SOUTH->pListHead;
				cellLoop(particle_i, node_j);

				node_j = cell->NEIGHBOURS.SOUTHEAST->pListHead;
				cellLoop(particle_i, node_j);

				node_j = cell->NEIGHBOURS.EAST->pListHead;
				cellLoop(particle_i, node_j);

				node_j = cell->NEIGHBOURS.NORTHEAST->pListHead;
				cellLoop(particle_i, node_j);

				// Rebase to next particle on next iteration
				node_i = node_i->next;
			}
		}
	}

	double WeeksChandlerAndersen::getVirial() {
		return this->virial;
	}

	double WeeksChandlerAndersen::getEnergy() {
		return this->energy;
	}

}
