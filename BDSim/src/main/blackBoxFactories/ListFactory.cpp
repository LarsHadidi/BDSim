#include <blackBoxFactories/ListFactory.h>


namespace blackBoxFactories {


	boost::property_tree::iptree*	ListFactory::initConf = nullptr;
	datastructures::Tesselation		ListFactory::grid;

	void ListFactory::initialize(boost::property_tree::iptree* initConf, datastructures::Tesselation grid){
		ListFactory::initConf = initConf;
		ListFactory::grid = grid;
	}



	datastructures::linkedParticleList* ListFactory::createLinkedParticleList(){
		BOOST_LOG_TRIVIAL(info) << "\t\tSetting up Linked-list.";
		// Allocate a continuous memory area for the linked list to treat it as an array
		datastructures::linkedParticleList* pList = new datastructures::linkedParticleList();
		pList->count = ListFactory::grid.size();
		pList->data = (datastructures::llnode<datastructures::Particle>*)calloc(pList->count, sizeof(datastructures::llnode<datastructures::Particle>));
		BOOST_LOG_TRIVIAL(info) << "\t\t\tAllocated " << (pList->count * sizeof(datastructures::llnode<datastructures::Particle>)) << " bytes for the linked-list.";
		// Set up each node
		BOOST_LOG_TRIVIAL(info) <<"\t\t\tTransfer of setup-grid data to linked list.";
		for (uint32_t i = 0; i < pList->count; ++i){
			double x = ListFactory::grid[i].x();
			double y = ListFactory::grid[i].y();
			pList->data[i].value = datastructures::Particle(x, y, 0, 0);
		}
		// Now every node contains a particle and a next-pointer to 0x0
		// return the base address of the array of nodes
		return pList;
	}

	datastructures::CellList* ListFactory::createCellList(){
		BOOST_LOG_TRIVIAL(info) << "\tSetting up cells.";
		datastructures::CellList* cellList = new datastructures::CellList();

		double cutoffRadius = std::sqrt(ListFactory::initConf->get<double>("squaredCutoffRadius"));
		double szX = ListFactory::initConf->get<double>("dimx");
		double szY = ListFactory::initConf->get<double>("dimy");

		uint32_t cols = std::floor(szX / cutoffRadius);
		uint32_t rows = std::floor(szY / cutoffRadius);

		double cellSizeX = szX / cols;
		double cellSizeY = szY / rows;

		assert(cellSizeX >= cutoffRadius);
		assert(cellSizeY >= cutoffRadius);

		cellList->cellCountX 	= cols;
		cellList->cellCountY 	= rows;
		cellList->cellCountTotal= rows * cols;
		cellList->cellSizeX 	= cellSizeX;
		cellList->cellSizeY 	= cellSizeY;

		cellList->data = (datastructures::Cell*)calloc(rows * cols, sizeof(datastructures::Cell));
		assert (cellList->data != NULL);


		BOOST_LOG_TRIVIAL(info) << "\t\tAllocated " << (rows * cols *  sizeof(datastructures::Cell)) << " bytes for the cell-list.";

		for(uint32_t v = 0; v < cellList->cellCountY; ++v){
			for(uint32_t u = 0; u < cellList->cellCountX; ++u){



				// Using conditionals for periodic boundary condition
				uint32_t south_u = 		u;
				uint32_t south_v = 		(v == 0)		? rows - 1 	: v - 1;

				uint32_t southEast_u = 	(u == cols -1) 	? 0 		: u + 1;
				uint32_t southEast_v = 	(v == 0) 		? rows - 1 	: v - 1;

				uint32_t east_u = 		(u == cols -1)	? 0 		: u + 1;
				uint32_t east_v = 		v;

				uint32_t northEast_u = (u == cols -1) 	? 0 		: u + 1;
				uint32_t northEast_v = (v == rows - 1) 	? 0 		: v + 1;

				uint64_t south_i 		= south_v 		* cols	+ south_u;
				uint64_t southEast_i	= southEast_v 	* cols 	+ southEast_u;
				uint64_t east_i			= east_v 		* cols	+ east_u;
				uint64_t northEast_i	= northEast_v 	* cols	+ northEast_u;

				// Initialize neighnour pointers
				datastructures::neighbourAddresses_s neighbourCells_stackAllocated =
				{
					&cellList->data[south_i],
					&cellList->data[southEast_i],
					&cellList->data[east_i],
					&cellList->data[northEast_i]
				};

				assert(neighbourCells_stackAllocated.SOUTH 		!= 0);
				assert(neighbourCells_stackAllocated.SOUTHEAST 	!= 0);
				assert(neighbourCells_stackAllocated.EAST		!= 0);
				assert(neighbourCells_stackAllocated.NORTHEAST	!= 0);

				// do some pointer arithmetics to write stack-allocated data into the heap where the cell list resides with its const pointers
				uint64_t i = v * cols + u;
				char* intraCellBaseAddress 	= (char*)&cellList->data[i];
				intraCellBaseAddress 		+= sizeof(datastructures::llnode<datastructures::Particle*>*);
				memcpy((void*)intraCellBaseAddress, &neighbourCells_stackAllocated, sizeof(datastructures::neighbourAddresses_s));

				assert(cellList->data[i].NEIGHBOURS.SOUTH 	 == neighbourCells_stackAllocated.SOUTH);
				assert(cellList->data[i].NEIGHBOURS.SOUTHEAST== neighbourCells_stackAllocated.SOUTHEAST);
				assert(cellList->data[i].NEIGHBOURS.EAST	 == neighbourCells_stackAllocated.EAST);
				assert(cellList->data[i].NEIGHBOURS.NORTHEAST== neighbourCells_stackAllocated.NORTHEAST);
			}
		}
		return cellList;
	}
}
