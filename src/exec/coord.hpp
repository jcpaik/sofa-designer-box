#ifndef COORD_HPP
#define COORD_HPP

#include <gmpxx.h>

namespace sofa_designer {
namespace geometry {

class Coord {
    public:
        mpq_class x, y;

        Coord() = default;
        Coord(const Coord &other) = default;
        Coord(Coord &&other) = default;
        // TODO (less important for running) : 
        // check each argument and 
        // pass rvalue ref if possible
        Coord(const mpq_class &x, const mpq_class &y) :
            x(x), y(y)
    {
    }
        ~Coord() = default;

        Coord &operator=(const Coord &other) = default;
        Coord &operator=(Coord &&other) = default;

        bool operator==(const Coord &other) const
        {
            return 
                (this->x == other.x) &&
                (this->y == other.y);
        }

        bool operator!=(const Coord &other) const
        {
            return !(*this == other);
        }

        mpq_class dot(const Coord &other) const
        {
            return x * other.x + y * other.y;
        }
};

std::ostream &operator<<(
        std::ostream &out, const Coord &coord);

}; // namespace geometry
}; // namespace sofa_designer

#endif
