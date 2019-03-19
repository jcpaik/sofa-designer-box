#include "coord.hpp"

#include <ostream>
#include <utility>

namespace sofa_designer {
namespace geometry {

std::ostream &operator<<(
        std::ostream &out, const Coord &coord)
{
    out << 
        "(" << coord.x << ", " << coord.y << ")";

    return out;
}

}; // namespace geometry
}; // namespace sofa_designer
