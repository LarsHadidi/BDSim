#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/// \file CellList
/// \brief The Linked List type

namespace datastructures {
	/// \brief Singly linked list datastructures.
	/// \tparam T Datatype of list items.
	template <typename T>
	struct llnode {
		llnode<T>* next; ///< Pointer to the next list item.
		T value; ///< Value of list item.
	};

	/// \brief Wrapper class for the linked list datatype.
	class linkedParticleList {
		public:
			uint32_t count; ///< Number of items in the list.
			llnode<Particle>* data; ///< Pointer to first list item.

			/// \brief Default constructor.
			/// Creates an uninitialized list
			linkedParticleList(){}
			/// \brief Copy constructor.
			/// Creates a copy of a list.
			/// \param rhs List to make a copy of.
			linkedParticleList(const linkedParticleList& rhs) {
				count = rhs.count;
				data = (llnode<Particle>*)malloc(count * sizeof(llnode<Particle>));
				memcpy(data, rhs.data, count * sizeof(llnode<Particle>));
			}
	};
}
