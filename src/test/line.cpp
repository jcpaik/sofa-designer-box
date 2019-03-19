#include "catch.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include <gmp.h>

#include "coord.hpp"
#include "line.hpp"

namespace sofa_designer {
namespace geometry {

TEST_CASE( "Basic functionality of Line", "[Line]" ) {
    // All initializers should work as expected
    Line l0;
    Line l1(1, 1);
    Line l2(l1);
    Line l3(Line(1, 1));
    Line l4(Coord(0, 1), Coord(2, 3));
    Line l5(Coord(-1, 1), 1);

    Line m0(Coord(0, 0), Coord(1, 1));
    REQUIRE(l0 != l1);
    REQUIRE(l0 != l2);
    REQUIRE(l0 != l3);
    REQUIRE(l0 != l4);
    REQUIRE(l0 != l5);
    REQUIRE(l1 == l2);
    REQUIRE(l1 == l3);
    REQUIRE(l1 == l4);
    REQUIRE(l1 == l5);
    REQUIRE(Line(Coord(-1, 1), Coord(1, 4))
         == Line(mpq_class(3, 2), mpq_class(5, 2)));
    REQUIRE(Line(Coord(-1, 1), Coord(1, 4))
         != Line(mpq_class(5, 2), mpq_class(3, 2)));

    std::vector<Line> lines = {
        Line(-1, -1),
        Line(-1, 0),
        Line(-1, 1),
        Line(0, -1),
        Line(0, 0),
        Line(0, 1),
        Line(1, -1),
        Line(1, 0),
        Line(1, 1),
    };
    std::vector<Line> expected_sorted_lines(lines);
    std::vector<Line> not_expected_sorted_lines = {
        Line(-1, 1),
        Line(-1, 0),
        Line(-1, -1),
        Line(0, 1),
        Line(0, 0),
        Line(0, -1),
        Line(1, 1),
        Line(1, 0),
        Line(1, -1),
    };
    std::shuffle(lines.begin(), lines.end(),
            std::default_random_engine(777));
    std::sort(lines.begin(), lines.end());
    REQUIRE(lines == expected_sorted_lines);
    REQUIRE(lines != not_expected_sorted_lines);
}

TEST_CASE( "Check Intersections of Lines", "[Line]" ) {
    REQUIRE(intersection(Line(-1, 0), Line(1, -1)) == 
            Coord(mpq_class("1/2"), mpq_class("-1/2")));
}

TEST_CASE( "Check Arrangements of Lines", "[Line]" ) {
    REQUIRE(
            arrangement(Line(-1, 0), Line(0, 1), Line(1, 0)) ==
            kU);
    REQUIRE(
            arrangement(Line(-1, 0), Line(0, 0), Line(1, 0)) ==
            kV);
}

}; // namespace geometry
}; // namespace sofa_designer
