#pragma once

#include <datastructures/Particle.h>
#include <datastructures/LinkedList.h>

/// \file CellList
/// \brief The Cell List type

namespace datastructures {
	struct Cell_s; // forward-declaration needed for neighbourAddress_s
	/// \brief Struct containing the addresses of the four neighbours of a cell.
	struct neighbourAddresses_s {
			struct Cell_s* const SOUTH;
			struct Cell_s* const SOUTHEAST;
			struct Cell_s* const EAST;
			struct Cell_s* const NORTHEAST;
	};

	/// \brief A cell pointing into a list node and holding the addresses of its neighbour cells.
	struct Cell_s {
		llnode<datastructures::Particle>* pListHead; ///< Pointer to the head of a singly linked list holding partciles.
		const struct neighbourAddresses_s NEIGHBOURS; ///< The addresses of the four neighbours of the cell.
	};

	typedef struct Cell_s Cell;

	/// \brief Wrapper type for the array of cells.
	struct CellList_s {
		uint32_t cellCountX; ///< Number of cells in x-dimension.
		uint32_t cellCountY; ///< Number of cells in y-dimension.
		uint64_t cellCountTotal; ///< Total number of cells.
		double cellSizeX; ///< Width of each cell.
		double cellSizeY; ///< Height of each cell.
		Cell* data; ///< Pointer to array of cells.
	};

	typedef struct CellList_s CellList;
}
