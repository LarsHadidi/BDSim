#pragma once

// Basic headers
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <limits>
// Utils
#include <random>
#include <chrono>
#include <ratio>
#include <time.h>
#include <tools/Logging.h>
#include <signal.h>
// Collections
#include <vector>
#include <array>
#include <unordered_set>
#include <algorithm>
// Math
#include <math.h>
// Boost lib
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/numeric/conversion/cast.hpp>
// Interfaces
#include <interfaces/Interfaces.h>
// Project Classes
#include <datastructures/Datastructures.h>
#include <interfaces/Interfaces.h>
#include <forcefields/ForceFields.h>
#include <integrators/Integrators.h>
#include <samplers/Samplers.h>
#include <blackBoxFactories/blackBoxFactories.h>
#include <VoronoiDecomposer/VoronoiDecomposer.h>

class Simulator {
	public:
		/// \brief Initializes the system and runs a benchmark.
		/// Sets up the system in following order:
		/// - Signal handlers
		/// - Given quantities such as temperature, timestep, etc.
		/// - Cutoff radius
		/// - Initial tesselation (the grid)
		/// - Cell list
		/// - Output folder
		/// - Voronoi decomposer
		/// - Forcefield
		/// - Integrator
		/// - Samplers
		/// And finally runs a benchmark after which all samplers will be reset.
		/// \param[in] initConf Pointer to property tree holding initial configuration data.
		static void initialize(boost::property_tree::iptree* initConf);
		/// \brief Starts the simulation.
		/// Starts to relax the system if a relaxation time greater than zero is given in the run-file.
		/// Subsequently the simulation will be performed.
		static void run();
		/// \brief Checks if there were particles lost during the simulation.
		/// \param[in] initConf Pointer to property tree holding initial configuration data.
		static void validateSystem(boost::property_tree::iptree* initConf);
		/// \brief Shuts down the system by freeing ressources and flushing remaining data to disk.
		static void tearDown();
	private:
		static datastructures::Tesselation grid; ///< The initial tesselation of the domain.
		static datastructures::CellList* cellList; ///< Pointer to the cell list.
		static datastructures::linkedParticleList* linkedParticleList; ///< Pointer to the continuous memory area holding the singly linked lists which contain the particles.
	    static datastructures::linkedParticleList* linkedParticleListSwapBuffer; ///< Pointer the another continuous memory area for the linked lists after address location reordering.
		static double timeStep; ///< Time inverval in the system's unit space for a simulation step.
		static double simulationTime; ///< Simulation time in the system's units.
		static double relaxationTime; ///< Relaxation time in the system's units.
		static double cacheLineOptRate; ///< Normed optimization rate of spation memory coherence. A value of one means optimizing every step (which is of poor performance).
		static uint32_t benchmarkSteps; ///< Number of iterations for the benchmark.
		static std::string outputPath; ///< Path to output folder.
		volatile static bool noAbort; ///< Interrupt flag.
		static interfaces::Integrator* integrator; ///<  Pointer to the integrator.
		static interfaces::ForceField* forceField; ///< Pointer to the forcefield.
		static std::vector<std::string> sensors; ///< Vector of names of samplers, for user.
		static VoronoiDecomposer::VoronoiDecomposer* voronoiDecomposer; ///< Pointer to the voronoi decomposer.
		static std::vector<interfaces::Sampler*>* samplers; ///< Pointer to vector containing pointers to samplers.
		static boost::property_tree::iptree* settings; ///< Pointer to property tree containing application settings.
		/// \brief Private constructor to enforce singleton usage.
		Simulator();
		/// \brief Sorts particles into their cells by updating the cell list pointers as well as the singly linked list pointers.
		/// Updates the pointers in the singly linked list which contains all particles such that particles within the same cell are connected.
		/// The partial list is terminated by a null pointer. The list head will be put as the new pointer into the corresponding cell.
		static void updateCellList();
		/// \brief Sets all particles' force values to zero.
		static void resetForces();
		/// \brief Initialize the grid using  the corresponding factory class.
		/// \param[in] initConf Pointer to inital configuration containing parsed user input
		static void initGrid(boost::property_tree::iptree* initConf);
		/// \brief Initialize the Cell-List using the corresponding factory class and allocate a swap buffer for cache optimization.
		/// \param[in] initConf Pointer to inital configuration containing parsed user input
		static void initCellList(boost::property_tree::iptree* initConf);
		/// \brief Initialize the forcefield using the corresponding factory class.
		/// \param[in] initConf Pointer to inital configuration containing parsed user input.
		static void initForceField(boost::property_tree::iptree* initConf);
		/// \brief Initialize the integrator using the corresponding factory class.
		/// \param[in] initConf Pointer to inital configuration containing parsed user input
		static void initIntegrator(boost::property_tree::iptree* initConf);
		/// \brief Initialize the output filestreams.
		/// \param[in] initConf Pointer to inital configuration containing parsed user input
		static void initOutput(boost::property_tree::iptree* initConf);
		/// \brief Initialize the samplers using the corresponding factory class.
		/// \param[in] initConf Pointer to inital configuration containing parsed user input
		static void initSamplers(boost::property_tree::iptree* initConf);
		/// \brief Reads the settings.ini file which is to be placed in the conf folder.
		/// The file contains general settings for the simulation software.
		static void readSettings();
		/// \brief Optimizes the linked list for the CPU cache.
		/// Copies the list nodes in the order which is defined by the list links into another memory location. Then swaps the buffers by simply
		/// swapping the start addresses of the buffers, therefore pointer-pointers are needed to propagate that change through the whole application.
		/// The swapped buffer then contains the nodes in the order the are supposed to be traversed, therefore improving spatial cache coherence.
		static void optimizeForCacheLine();
		/// \brief Callback function to set the abort flag.
		/// \praram[in] Signal value
		static void signalHandler(int signal);
		/// \brief Performs a benchmark on the set up system and reporting the results.
		/// Can be used to estimate the running time and to decide which code path should be taken when it comes to parallel processing over distributed nodes.
		/// If an ICMP Echo request takes much longer than a sychronized iteration, distributed calculations will decrease performance.
		static void benchmark();
		/// \brief Relaxes the system, not sampling anything.
		/// Propagates the system over a certain period of time, given by the initial configuration.
		static void relax();
		/// \brief Runs the simulation and samples data.
		static void simulate();
};
