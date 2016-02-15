#include <simulator/Simulator.h>


// ---------------------------------------------------------------
// -                    Initialize static members                -
// ---------------------------------------------------------------

datastructures::Tesselation Simulator::grid;
datastructures::CellList* Simulator::cellList 								= nullptr;
datastructures::linkedParticleList* Simulator::linkedParticleList			= nullptr;
datastructures::linkedParticleList* Simulator::linkedParticleListSwapBuffer = nullptr;
double Simulator::timeStep 			= 0;
double Simulator::simulationTime 	= 0;
double Simulator::relaxationTime 	= 0;
double Simulator::cacheLineOptRate	= 0;
uint32_t Simulator::benchmarkSteps  = 0;
std::string Simulator::outputPath = "";
volatile bool Simulator::noAbort = true;

interfaces::Integrator* Simulator::integrator = nullptr	;
interfaces::ForceField* Simulator::forceField = nullptr;
std::vector<std::string> Simulator::sensors = { };
VoronoiDecomposer::VoronoiDecomposer* Simulator::voronoiDecomposer = nullptr;
std::vector<interfaces::Sampler*>* Simulator::samplers = new std::vector<interfaces::Sampler*>();

boost::property_tree::iptree* Simulator::settings = new boost::property_tree::iptree();

/// Initialize the simulation grid.
/// @param initConf Reference to inital configuration struct containing parsed user input
void Simulator::initGrid(boost::property_tree::iptree* initConf) {
	BOOST_LOG_TRIVIAL(info)<< "Initializing setup-grid.";

	blackBoxFactories::GridFactory::initialize(initConf);
	if (initConf->get<std::string>("initstate.tesselation").compare("QUAD") == 0) {
		Simulator::grid = blackBoxFactories::GridFactory::createQuadGrid();
	}
	else if (initConf->get<std::string>("initstate.tesselation").compare("HEXA") == 0) {
		Simulator::grid = blackBoxFactories::GridFactory::createHexaGrid();
	}

	BOOST_LOG_TRIVIAL(info)<< "Setup-grid initialized.";
}


void Simulator::initCellList(boost::property_tree::iptree* initConf) {
	BOOST_LOG_TRIVIAL(info) << "Initializing Cell-List.";
	blackBoxFactories::ListFactory::initialize(initConf, Simulator::grid);
	BOOST_LOG_TRIVIAL(info) << "\tCreating linked list for particles.";
	Simulator::linkedParticleList = blackBoxFactories::ListFactory::createLinkedParticleList();
	BOOST_LOG_TRIVIAL(info) << "\tCreating linked list swap buffer.";
	// Swap Buffer will be an exact copy
	Simulator::linkedParticleListSwapBuffer = blackBoxFactories::ListFactory::createLinkedParticleList();
	BOOST_LOG_TRIVIAL(info) << "\tCreating cell list.";
	Simulator::cellList = blackBoxFactories::ListFactory::createCellList();
	Simulator::updateCellList();
	BOOST_LOG_TRIVIAL(info) << "Cell-List initialized.";
}


void Simulator::initForceField(boost::property_tree::iptree* initConf) {
	BOOST_LOG_TRIVIAL(info)<< "Initializing forcefield.";
	blackBoxFactories::ForceFieldFactory::initialize(initConf, Simulator::cellList);
	Simulator::forceField = blackBoxFactories::ForceFieldFactory::createWeeksChandlerAndersonForceField();

	BOOST_LOG_TRIVIAL(info)<< "Forcefield initialized.";
}


void Simulator::initIntegrator(boost::property_tree::iptree* initConf) {
	BOOST_LOG_TRIVIAL(info)<< "Initializing integrator.";
	blackBoxFactories::IntegratorFactory::initialize(initConf, &(Simulator::linkedParticleList));
	Simulator::integrator = blackBoxFactories::IntegratorFactory::createEulerMaruyamaIntegrator();

	BOOST_LOG_TRIVIAL(info)<< "Integrator initialized.";
}


void Simulator::initOutput(boost::property_tree::iptree* initConf) {
	try {
		// Create output folder
		BOOST_LOG_TRIVIAL(info) << "Creating output folder.";

		boost::filesystem::path path(initConf->get<std::string>("outputpath"));

		if(!boost::filesystem::exists(path)){
			boost::filesystem::create_directory(path);
		}

		Simulator::outputPath = path.string();

		// Create sidecar file
		boost::property_tree::iptree params;
		params.put("Parameters.particles", initConf->get<uint32_t>("particles"));
		params.put("Parameters.dimx", initConf->get<double>("dimx"));
		params.put("Parameters.dimy", initConf->get<double>("dimy"));
		params.put("Parameters.timestep", initConf->get<double>("timestep"));
		params.put("Parameters.simtime", initConf->get<double>("simtime"));
		params.put("Parameters.relaxation", initConf->get<double>("relaxation"));
		params.put("Parameters.temperature", initConf->get<double>("temperature"));

		boost::property_tree::write_ini((boost::filesystem::path(Simulator::outputPath)/"params").string(), params);
	}
	catch (boost::filesystem::filesystem_error& e){
		BOOST_LOG_TRIVIAL(error) << "Could not initialize output directory: " << e.what();
		exit(1);
	}
	catch (std::ofstream::failure& e) {
		BOOST_LOG_TRIVIAL(error) << "Could not write to output directory: " << e.what();
		exit(1);
	}
}


void Simulator::initSamplers(boost::property_tree::iptree* initConf) {

	BOOST_LOG_TRIVIAL(info)<< "Initializing samplers.";

	blackBoxFactories::SamplerFactory::initialize(initConf, &(Simulator::linkedParticleList), Simulator::cellList,Simulator::forceField, Simulator::voronoiDecomposer);
	Simulator::samplers->reserve(Simulator::sensors.size());
	for(std::string sensor : Simulator::sensors) {
		if (sensor.compare("CONFS") == 0){
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createConfigurationsSampler();
			Simulator::samplers->push_back(sampler);
		}
		else if(sensor.compare("PRESSURE") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createPressureSampler();
			Simulator::samplers->push_back(sampler);
		}
		else if (sensor.compare("ENERGY") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createEnergySampler();
			Simulator::samplers->push_back(sampler);
		}
		else if (sensor.compare("VIZ") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createVisualizationSampler();
			Simulator::samplers->push_back(sampler);
		}
		else if (sensor.compare("DEFECTS") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createDefectsSampler();
			Simulator::samplers->push_back(sampler);
		}
		else if (sensor.compare("DEFECT-DENSITIES") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createDefectDensitiesSampler();
			Simulator::samplers->push_back(sampler);
		}
		else if (sensor.compare("PSI6") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createPSI6Sampler();
			Simulator::samplers->push_back(sampler);
		}
		else if (sensor.compare("GLOBAL-PSI6") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createGlobalPSI6Sampler();
			Simulator::samplers->push_back(sampler);
		}
		else if(sensor.compare("VORO-CELLS") == 0) {
			interfaces::Sampler* sampler = blackBoxFactories::SamplerFactory::createVoroCellsSampler();
			Simulator::samplers->push_back(sampler);
		}
	}
	BOOST_LOG_TRIVIAL(info)<< "Samplers initialized.";

}

void Simulator::initialize(boost::property_tree::iptree* initConf) {
	if (signal((int)SIGINT, Simulator::signalHandler)  == SIG_ERR) BOOST_LOG_TRIVIAL(error) << "Error setting up signal handler for SIGINT";
	if (signal((int)SIGTSTP, Simulator::signalHandler) == SIG_ERR) BOOST_LOG_TRIVIAL(error) << "Error setting up signal handler for SIGTSTP";
	if (signal((int)SIGTERM, Simulator::signalHandler) == SIG_ERR) BOOST_LOG_TRIVIAL(error) << "Error setting up signal handler for SIGTERM";

	Simulator::timeStep = initConf->get<double>("timestep");
	Simulator::relaxationTime = initConf->get<double>("relaxation");
	Simulator::simulationTime = initConf->get<double>("simtime");
	boost::optional<boost::property_tree::iptree&> node = initConf->get_child("sensors");

	BOOST_FOREACH(boost::property_tree::iptree::value_type& v, *node) {
		Simulator::sensors.push_back(v.second.data());
	}

	Simulator::readSettings();

	initConf->put("squaredCutoffRadius", pow(2.0, 1.0 / 3.0));

	initGrid(initConf); //must be called first as it sets further init parameters on which other initializations rely
	initCellList(initConf);
	initOutput(initConf);

	Simulator::voronoiDecomposer = new VoronoiDecomposer::VoronoiDecomposer(initConf, &(Simulator::linkedParticleList));

	initForceField(initConf);
	initIntegrator(initConf);
	initSamplers(initConf);

	if(Simulator::benchmarkSteps != 0) {
		BOOST_LOG_TRIVIAL(info) << "*** Starting benchmark ***";
		Simulator::benchmark();
	}

	BOOST_LOG_TRIVIAL(info)<< "Initialization complete.\n";
}

void Simulator::updateCellList(){

	datastructures::llnode<datastructures::Particle>* pNode = nullptr;

	for(uint64_t i = 0; i < Simulator::cellList->cellCountTotal; ++i){
		Simulator::cellList->data[i].pListHead = 0x0;
	}

	for(uint32_t i = 0; i < Simulator::linkedParticleList->count; ++i){
		// (u,v) yields the cell into which the particle i should be sorted into, indices are based on zero.
		// The cast to unsigend int yields the correct cell index

		pNode = Simulator::linkedParticleList->data + i;
		const uint32_t u = pNode->value.posX / Simulator::cellList->cellSizeX;
		const uint32_t v = pNode->value.posY / Simulator::cellList->cellSizeY;

		const uint64_t cellID = v * Simulator::cellList->cellCountX + u;
		if(cellID < Simulator::cellList->cellCountTotal){
			// Set the node's next pointer to the previous head
			pNode->next = Simulator::cellList->data[cellID].pListHead;
			// Then set the new head of the cell-list
			Simulator::cellList->data[cellID].pListHead = pNode;
		}
	}
}

inline void Simulator::resetForces(){
	for(uint32_t i = 0; i < Simulator::linkedParticleList->count; ++i){
		Simulator::linkedParticleList->data[i].value.forceX = 0;
		Simulator::linkedParticleList->data[i].value.forceY = 0;
	}
}

inline void Simulator::benchmark(){
	datastructures::linkedParticleList* addressCache = Simulator::linkedParticleList;
	Simulator::linkedParticleList = new datastructures::linkedParticleList(*(Simulator::linkedParticleList));

	const double CL_OPT_THRESHOLD = 1 / Simulator::cacheLineOptRate;
	const uint32_t BENCHMARK_STEPS = Simulator::benchmarkSteps;

	size_t cumulativeForceCalculationDuration = 0;
	size_t cumulativeIntegrationDuration = 0;
	size_t cumulativeUpdateDuration = 0;
	size_t* cumulativeSamplerDuration = (size_t*)calloc(Simulator::samplers->size(), sizeof(size_t));

	std::chrono::high_resolution_clock::time_point timePoint;
	std::chrono::high_resolution_clock::duration duration;
	typedef std::chrono::microseconds timeunit;

	BOOST_LOG_TRIVIAL(info) << "Measuring timings of simulator.";
	double j = 0;
	uint32_t n = 0;
	while(noAbort && n < BENCHMARK_STEPS) {
		timePoint = std::chrono::high_resolution_clock::now();
		Simulator::resetForces();
		Simulator::forceField->calcForces();
		duration = std::chrono::high_resolution_clock::now() - timePoint;
		cumulativeForceCalculationDuration += std::chrono::duration_cast<timeunit>(duration).count();

		timePoint = std::chrono::high_resolution_clock::now();
		Simulator::integrator->integrate();
		duration = std::chrono::high_resolution_clock::now() - timePoint;
		cumulativeIntegrationDuration += std::chrono::duration_cast<timeunit>(duration).count();

		timePoint = std::chrono::high_resolution_clock::now();
		Simulator::updateCellList();
		duration = std::chrono::high_resolution_clock::now() - timePoint;
		cumulativeUpdateDuration += std::chrono::duration_cast<timeunit>(duration).count();

		for(int i = 0; i < Simulator::samplers->size(); ++i){
			timePoint = std::chrono::high_resolution_clock::now();
			Simulator::samplers->at(i)->sample();
			duration = std::chrono::high_resolution_clock::now() - timePoint;
			cumulativeSamplerDuration[i] += std::chrono::duration_cast<timeunit>(duration).count();
		}
		Simulator::voronoiDecomposer->finalize();
		if(++j >= CL_OPT_THRESHOLD){
			j = 0;
			Simulator::optimizeForCacheLine();
		}
		++n;
	}
	if(noAbort) {
		double averageForceCalculationDuration = (double) cumulativeForceCalculationDuration / BENCHMARK_STEPS;
		double averageIntegrationDuration = (double) cumulativeIntegrationDuration / BENCHMARK_STEPS;
		double averageUpdateDuration = (double) cumulativeUpdateDuration / BENCHMARK_STEPS;
		double averageSamplerDuration[Simulator::samplers->size()];
		double averageBaseDuration = averageForceCalculationDuration + averageIntegrationDuration + averageUpdateDuration;
		double averageTotalDuration = averageBaseDuration;
		for (int i = 0; i < Simulator::samplers->size(); ++i) {
			averageSamplerDuration[i] = (double) cumulativeSamplerDuration[i] / BENCHMARK_STEPS;
			averageTotalDuration += averageSamplerDuration[i];
		}

		std::stringstream sstream;
		BOOST_LOG_TRIVIAL(info) << "Benchmark results are: ";
		if (averageForceCalculationDuration == 0) {
			sstream << "negligible";
		} else {
			sstream << averageForceCalculationDuration << " µs";
		}
		BOOST_LOG_TRIVIAL(info) << "\tAverage time needed for force calculation: " << sstream.str();

		sstream.str(std::string());
		if (averageIntegrationDuration == 0) {
			sstream << "negligible";
		} else {
			sstream << averageIntegrationDuration << " µs";
		}
		BOOST_LOG_TRIVIAL(info) << "\tAverage time needed for integration: " << sstream.str();

		sstream.str(std::string());
		if (averageUpdateDuration == 0) {
			sstream << "negligible";
		} else {
			sstream << averageUpdateDuration << " µs";
		}
		BOOST_LOG_TRIVIAL(info) << "\tAverage time needed for updating cell list: " << sstream.str();

		for (int i = 0; i < Simulator::sensors.size(); ++i) {
			sstream.str(std::string());
			if (averageSamplerDuration[i] == 0) {
				sstream << "negligible";
			} else {
				sstream << averageSamplerDuration[i] << " µs";
			}
			BOOST_LOG_TRIVIAL(info) << "\tAverage time needed for sampling on " << Simulator::sensors.at(i) << ": " <<
									sstream.str();
		}

		sstream.str(std::string());
		if (averageBaseDuration != 0) {
			std::chrono::duration<double, std::micro> estimatedRuntime((averageBaseDuration *  Simulator::relaxationTime + averageTotalDuration * Simulator::simulationTime) / Simulator::timeStep);
			sstream <<  std::chrono::duration<double, std::ratio<60, 1>>(estimatedRuntime).count() << " minutes";
			BOOST_LOG_TRIVIAL(info) << "\tEstimated total runtime: " << sstream.str();
		}

		BOOST_LOG_TRIVIAL(info) << "Restoring to initial particle configuration";
		Simulator::linkedParticleList = addressCache;
		Simulator::updateCellList();
		BOOST_LOG_TRIVIAL(info) << "Resetting sensors";
		for (interfaces::Sampler *sampler : *Simulator::samplers) {
			sampler->reset();
		}
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "Benchmark aborted";
	}
}

inline void Simulator::relax(){
	BOOST_LOG_TRIVIAL(info)<< "Starting relaxation:\t" <<  Simulator::relaxationTime / Simulator::timeStep << " steps.";
	const double CL_OPT_THRESHOLD = 1 / Simulator::cacheLineOptRate;
	double t = 0;
	double i = 0;
	const double TIME = Simulator::relaxationTime;
	const double TIME_STEP = Simulator::timeStep;
	while(noAbort && t < TIME) {

		Simulator::resetForces();
		Simulator::forceField->calcForces();
		Simulator::integrator->integrate();
		Simulator::updateCellList();

#ifdef _DEBUG
for(interfaces::Sampler* sampler : *Simulator::samplers){
	sampler->sample();
}
Simulator::voronoiDecomposer->finalize();
#endif

		if(++i >= CL_OPT_THRESHOLD){
			i = 0;
			Simulator::optimizeForCacheLine();
		}
		t += TIME_STEP;
	}
	if (noAbort) BOOST_LOG_TRIVIAL(info)<< "Relaxation finished. Floating point discrepancy: " << (t - Simulator::relaxationTime) / Simulator::timeStep << " steps.";
}

inline void Simulator::simulate(){
	const double CL_OPT_THRESHOLD = 1 / Simulator::cacheLineOptRate;
	BOOST_LOG_TRIVIAL(info)<< "Starting simulation:\t" << Simulator::simulationTime / Simulator::timeStep << " steps.";
	double t = 0;
	double i = 0;
	const double TIME = Simulator::simulationTime;
	const double TIME_STEP = Simulator::timeStep;
	while(noAbort && t < TIME) {

		Simulator::resetForces();
		Simulator::forceField->calcForces();
		Simulator::integrator->integrate();
		Simulator::updateCellList();
		for(interfaces::Sampler* sampler : *Simulator::samplers){
			sampler->sample();
		}
		Simulator::voronoiDecomposer->finalize();
		// Cache-Line optimization rearranges the particle order, so it may not be performed between a Voronoi-Decomposition and a corresponding sampling
		// which relies on the indices in the linkedParticleList to correspond to the Voronoi-Container particle's indices
		if(++i >= CL_OPT_THRESHOLD){
			i = 0;
			Simulator::optimizeForCacheLine();
		}
		t += TIME_STEP;
	}
	if (noAbort) BOOST_LOG_TRIVIAL(info)<< "Simulation finished. Floating point discrepancy: " << (t - Simulator::simulationTime) / Simulator::timeStep << " steps.";
}

// ---------------------------------------------------------------
// -                      Start the simulation                   -
// ---------------------------------------------------------------
void Simulator::run() {
	BOOST_LOG_TRIVIAL(info) << "*** Starting simulation ***\n";
	BOOST_LOG_TRIVIAL(info) << "The simulation will take approximately " << (Simulator::relaxationTime + Simulator::simulationTime) / Simulator::timeStep << " steps.";
	bool simulationPhase = false;
	if(noAbort && Simulator::relaxationTime != 0) {
		Simulator::relax();
	}
	if (noAbort) {
		simulationPhase = true;
		Simulator::simulate();
	}
	if (noAbort) {
		BOOST_LOG_TRIVIAL(info) << "Simulation finished\n";
	}
	else {
		if (simulationPhase) {
			BOOST_LOG_TRIVIAL(info) << "Simulation aborted.";
		}
		else {
			BOOST_LOG_TRIVIAL(error) << "Program stopped without entering simulation phase.";
		}
	}
}

void Simulator::validateSystem(boost::property_tree::iptree* initConf) {
	BOOST_LOG_TRIVIAL(info) << "Checking for lost particles.";
	datastructures::Particle *particle;
	uint32_t losses = 0;
	for (uint32_t n = 0; n < Simulator::linkedParticleList->count; ++n) {
		particle = &(Simulator::linkedParticleList->data[n].value);

		// particle still out of box after last integration step using periodic boundary conditions
		if (particle->posX < 0 || particle->posY < 0 || particle->posX > initConf->get<double>("dimx") || particle->posY > initConf->get<double>("dimy")) {
			++losses;
		}
	}
	if (losses > 0) {
		BOOST_LOG_TRIVIAL(error) << "There were " << losses << " particles lost.";
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "No particles lost.";
	}
}

void Simulator::readSettings(){
	try {
		boost::property_tree::ini_parser::read_ini((boost::filesystem::path("conf") / "settings.ini").string(), *(Simulator::settings));
		Simulator::cacheLineOptRate = Simulator::settings->get<double>("Optimization.cacheLineOptRate");
		Simulator::benchmarkSteps = Simulator::settings->get<uint32_t>("Benchmark.steps");
	}
	catch(const boost::property_tree::ptree_error &e) {
		BOOST_LOG_TRIVIAL(error) << e.what();
	}

	if(Simulator::cacheLineOptRate == 0){
		BOOST_LOG_TRIVIAL(info) << "No cache optimization.";
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "Cacheline optimization rate: " << Simulator::cacheLineOptRate;
	}
	if(Simulator::benchmarkSteps == 0){
		BOOST_LOG_TRIVIAL(error) << "No benchmark.";
		exit(1);
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "Banchmark steps: " << Simulator::benchmarkSteps;
	}
}

// Align the linked list
void Simulator::optimizeForCacheLine(){
	uint32_t swpPos = 0;
	uint32_t swpBasePos = 0;
	for(uint64_t i = 0; i < Simulator::cellList->cellCountTotal; ++i) {
		swpBasePos = swpPos;
		datastructures::llnode<datastructures::Particle> *node_i = Simulator::cellList->data[i].pListHead;
		while (node_i) {
			// Copy into swap buffer
			// First copy particle into the buffer-node
			Simulator::linkedParticleListSwapBuffer->data[swpPos].value = node_i->value;
			// Now, as the the swap buffer will contain the llnodes in a linear order,
			// set the swap node's next pointer to the follow-up node, if and only if
			// it is not a termination node (i.e. next points to null)
			if (node_i->next) {
				Simulator::linkedParticleListSwapBuffer->data[swpPos].next = Simulator::linkedParticleListSwapBuffer->data + swpPos + 1;
			}
			else {
				Simulator::linkedParticleListSwapBuffer->data[swpPos].next = 0x0;
			}
			++swpPos;
			node_i = node_i->next;
		}
		// Update the cell-list but also preserve information about empty cells
		if(node_i) {
			Simulator::cellList->data[i].pListHead = Simulator::linkedParticleListSwapBuffer->data + swpBasePos;
		}
		else {
			Simulator::cellList->data[i].pListHead = 0x0;
		}
	}
	// Swap with buffer
	datastructures::linkedParticleList* tmp = Simulator::linkedParticleList;
	Simulator::linkedParticleList = Simulator::linkedParticleListSwapBuffer;
	Simulator::linkedParticleListSwapBuffer = tmp;
}
// ---------------------------------------------------------------
// -                   Function to clean up memory               -
// ---------------------------------------------------------------
void Simulator::tearDown() {
	BOOST_LOG_TRIVIAL(info) << "*** Tearing down Simulation ***";

		delete Simulator::settings;

	BOOST_LOG_TRIVIAL(info) << "\tDeallocating integrator.";
	delete Simulator::integrator;

	BOOST_LOG_TRIVIAL(info) << "\tDeallocating forcefield.";
	delete Simulator::forceField;

	BOOST_LOG_TRIVIAL(info) << "Removing sensors.";
	for(interfaces::Sampler* sampler : *Simulator::samplers) delete sampler;
	Simulator::samplers->clear();
	delete Simulator::samplers;

	BOOST_LOG_TRIVIAL(info) << "Deallocating Cell-List.";
	free(Simulator::cellList->data);

	BOOST_LOG_TRIVIAL(info) << "Deallocating Linked-List.";
	free(Simulator::linkedParticleList->data);
	free(Simulator::linkedParticleListSwapBuffer->data);
}

void Simulator::signalHandler(int signal){
	Simulator::noAbort = false;
}
