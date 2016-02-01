#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <vector>


/// \file Tesselation
/// \brief The tesselation type
namespace datastructures {
    typedef std::vector<boost::geometry::model::d2::point_xy<double>> Tesselation; ///< A vector holding 2D-Points which represent a tesselation of the domain.
}