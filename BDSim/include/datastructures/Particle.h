#pragma once

/// \file Particle
/// \brief The particle type

namespace datastructures {
	/// \brief Struct holding particle data.
	struct Particle_s {
		double posX; ///< X-Position of the particle.
		double posY; ///< Y-Position of the particle.
		double forceX; ///< Force in x-direction acting on the particle.
		double forceY; ///< Force in y-direction acting on the particle.
		/// \brief Particle constructor.
		/// \param x X-Position of the particle
		/// \param x Y-Position of the particle
		/// \param x Initial force in x-direction acting on the particle.
		/// \param x Initial force in y-direction acting on the particle.
		Particle_s (double x,double y,double fX, double fY) : posX(x), posY(y), forceX(fX), forceY(fY) {}
	};
	typedef struct Particle_s Particle;
}
