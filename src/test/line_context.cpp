#include "catch.hpp"

#include <algorithm>
#include <random>
#include <vector>
#include <iostream>

#include <gmp.h>
#include <gmpxx.h>

#include "coord.hpp"
#include "line.hpp"
#include "line_context.hpp"

namespace sofa_designer {
namespace geometry {

TEST_CASE( "Basic functionality of VanillaLineContext", "[VanillaLineContext]" ) {
    std::vector<Line> lines = {
        Line(-1, -1),  // 0
        Line(-1, 1),
        Line(1, 1),
        Line(-1, 0),  // 1
        Line(-1, 1), // 2
        Line(0, -1),   // 3
        Line(0, 0),   // 4
        Line(-1, -1),
        Line(0, 1),  // 5
        Line(1, -1),   // 6
        Line(1, 0),   // 7
        Line(1, 1),  // 8
    };

    VanillaLineContext ctx(lines);
    REQUIRE(arrangement(
            ctx.all_lines()[1],
            ctx.all_lines()[4],
            ctx.all_lines()[6]) == kU);

    SECTION( "Check Arrangements of Lines in LineContext" ) {
        REQUIRE(ctx.arrangement(1, 2, 3) == kV);
        REQUIRE(ctx.arrangement(1, 2, 5) == kV);
        REQUIRE(ctx.arrangement(1, 5, 7) == kU);
        REQUIRE(ctx.arrangement(1, 4, 7) == kV);
        REQUIRE(ctx.arrangement(1, 4, 6) == kU);
        REQUIRE(ctx.arrangement(1, 3, 6) == kV);
    }
}

}; // namespace geometry
}; // namespace sofa_designer
