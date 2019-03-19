#include "catch.hpp"

#include "coord.hpp"

namespace sofa_designer {
namespace geometry {

TEST_CASE( "Basic functionality of Coord", "[Coord]" ) {

    // All initializers should work as expected
    Coord a;
    Coord b(1, 2), c(1, 2);
    Coord d(Coord(1, 2));  // rvalue reference

    REQUIRE(a == a);
    REQUIRE(a != b);
    REQUIRE(a != c);
    REQUIRE(a != d);
    REQUIRE(b != a);
    REQUIRE(b == b);
    REQUIRE(b == c);
    REQUIRE(b == d);

    a = b;
    REQUIRE(a == b);
    a.x++;
    REQUIRE(a != b);
    c = Coord(3, 4);
    d.x = 3;
    d.y = 4;
    REQUIRE(c == d);
    c.x = 4;
    REQUIRE(c != d);
    REQUIRE(Coord(4, 4) == c);
}

}; // namespace geometry
}; // namespace sofa_designer
