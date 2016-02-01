#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace geometry {
    typedef boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> Point2D;
}