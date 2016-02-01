#pragma once

#include <datastructures/Datastructures.h>
#include <blackBoxFactories/PRNGEngineFactory.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <stdint.h>
#include <unordered_set>
#include <vector>
#include <assert.h>

namespace blackBoxFactories {
    /// \brief Class providing factory methods to create tesselations of the simulation domain.
    /// \author Lars Hadidi
    class GridFactory {
        public:
            /// \brief Initialize the Factory.
            /// \param[in] initConf Pointer to the initialization object.
            static void initialize(boost::property_tree::iptree* initConf);
            /// \brief Creates a quad tiled grid.
            /// \return List of 2D points representing the tiling.
            static datastructures::Tesselation createQuadGrid();
        /// \brief Creates a hexagonal tiled grid.
        /// \return List of 2D points representing the tiling.
        static datastructures::Tesselation createHexaGrid();
        private:

            static boost::property_tree::iptree *initConf;///< Pointer to the initializaton object

            /// \brief Removes a certain amount of points from a given list of 2D-Points as a consecutive block.
            /// The tesselation will be modified. The function removes the given amount of points from the end of the list.
            /// \param[in] The list of 2D-Points from which to remove the given amount of points
            /// \praram[in] Amount of points to be removed
            static void uniformRemoveSpill(datastructures::Tesselation& tesselation, uint32_t spill);

            /// \brief Removes a certain amount of points from a given list of 2D-Points at random positions with that list.
            /// The tesselation will be modified. The function removes the given amount of points at random positions in the list.
            /// \param[in] The list of 2D-Points from which to remove the given amount of points
            /// \praram[in] Amount of points to be removed
            static void scatteredRemoveSpill(datastructures::Tesselation& tesselation, uint32_t spill);
    };
}