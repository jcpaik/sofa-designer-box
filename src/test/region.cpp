#include "catch.hpp"

#include <algorithm>
#include <random>
#include <vector>
#include <iostream>

#include "coord.hpp"
#include "line.hpp"
#include "line_context.hpp"
#include "region.hpp"

namespace sofa_designer {
namespace geometry {

// Given a polygon as list of lines,
// rotate the list to form a unique, lexicographically smallest sequence
void sort_polygon(Polygon &poly)
{
    Polygon copy(poly);
    std::size_t len = poly.size();

    for (std::size_t i = 0; i < len; i++) {
        if (poly > copy)
            poly = copy;
        std::rotate(copy.begin(), copy.begin() + 1, copy.end());
    }
}

// Given list of polygons, find the unique ordered list
void sort_polygons(Polygons &polys)
{
    for (auto &poly : polys) {
        sort_polygon(poly);
    }

    sort(polys.begin(), polys.end());
}

void check_eq(Polygon p0, Polygon p1)
{
    sort_polygon(p0);
    sort_polygon(p1);
    REQUIRE(p0 == p1);
}

void check_eq(Polygons p0, Polygons p1)
{
    sort_polygons(p0);
    sort_polygons(p1);
    REQUIRE(p0 == p1);
}

TEST_CASE( "Test polygon sorters", "[sort_polygon][sort_polygons]" ) {

    Polygon poly = {4, 5, 3, 0, 7, 1, 5, 2, 0, 6};
    Polygon expected_poly = 
        {0, 6, 4, 5, 3, 0, 7, 1, 5, 2};
    check_eq(poly, expected_poly);
    sort_polygon(poly);
    REQUIRE(poly == expected_poly);

    Polygons polys = {
        {2, 7, 1, 5}, {6, 2, 0}
    };
    Polygons expected_polys = {
        {0, 6, 2}, {1, 5, 2, 7}
    };
    check_eq(polys, expected_polys);
    sort_polygons(polys);
    REQUIRE(polys == expected_polys);
}


TEST_CASE( "Initialize and run a HalfPlaneRegion", "[VanillaLineContext][HalfPlaneRegion]" ) {

    /*

       Start from *

            7         444444 
            70       5    6
           7  0     5     6
           7   03335     6
          7              6 
          7             6
         7              6
         7             6
        7      2222    6
        7     5    0  6
       7     5      0 6
       *11111        0

     */

    std::vector<Coord> verts = {
        Coord(-2, -1), Coord(-1, -1), Coord(0, 0), Coord(1, 0), Coord(2, -1),
        Coord(3, 2), Coord(2, 2), Coord(1, 1), Coord(0, 1), Coord(-1, 2)
    };

    INFO("Verticies received");
    CAPTURE(verts);

    std::vector<Line> lines;
    for (std::size_t i = 0; i < verts.size(); i++) {
        const Coord &p = verts[i];
        const Coord &q = verts[(i + 1) % verts.size()];
        lines.push_back(Line(p, q));
    }

    INFO("Lines generated");
    CAPTURE(lines);

    VanillaLineContext ctx(lines);
    std::vector<LineId> line_ids(lines.size());
    for (std::size_t i = 0; i < lines.size(); i++) {
        const Line &l = lines[i];
        int id = 0;
        while (id < ctx.num_lines() && ctx.line(id) != l)
            id++;
        REQUIRE (id != ctx.num_lines());

        const Coord &p = verts[i];
        const Coord &q = verts[(i + 1) % verts.size()];
        // As the line ids should have signs,
        if (p.x < q.x)
            line_ids[i] = id;
        else
            line_ids[i] = ~id;
    }
    INFO("Converting lines to id's");
    CAPTURE(line_ids);
    std::vector<LineId> expected_line_ids = { 
        1, 5, 2, 0, 6, 
        ~short(4), ~short(5), ~short(3), ~short(0), ~short(7)
    };
    REQUIRE(line_ids == expected_line_ids);

    INFO("Setup done");

    INFO("Initialize HalfPlaneRegions");
    HalfPlaneRegion r5(ctx, 5);

    /*

       Start from *

            7         444444 
            70       5    6
           7  0     5     6
           7   03335     6
          7              6 
          7             6
         7              6
         7             6
        7      2222    6
        7     5    0  6
       7     5      0 6
       *11111        0

     */

    // maybe TODO: refactor the combination of tests in one file
    // maybe TODO: given a polygon, generate auto tests

    REQUIRE(r5.contains_intersection(3, 0));
    REQUIRE(r5.contains_intersection(0, 3));
    REQUIRE(r5.contains_intersection(~short(0), ~short(3)));
    REQUIRE(r5.contains_intersection(~short(3), 0));
    REQUIRE(r5.contains_intersection(3, ~short(0)));

    REQUIRE(!r5.contains_intersection(0, 2));
    REQUIRE(!r5.contains_intersection(~short(0), ~short(2)));
    // no points on the line 
    REQUIRE(!r5.contains_intersection(5, 0));
    REQUIRE(!r5.contains_intersection(5, 3));
    REQUIRE(!r5.contains_intersection(7, 5));
    REQUIRE(!r5.contains_intersection(~short(0), 5));
    REQUIRE(!r5.contains_intersection(5, 6));

    REQUIRE(r5.contains_intersection(0, 7));
    REQUIRE(!r5.contains_intersection(0, 6));

    check_eq(r5.intersection(line_ids), {
        {5, ~short(3), ~short(0), ~short(7), 1},
    });

    // Found couple bugs using this
    // This test have two polygons cut out, and one edge of polygon
    // is aligned to the cutting line
    HalfPlaneRegion rn2(ctx, ~short(2));
    check_eq(rn2.intersection(line_ids), {
        {~short(2), ~short(7), 1, 5},
        {0, 6, ~short(2)}
    });

    HalfPlaneRegion r3(ctx, 3);
    check_eq(r3.intersection(line_ids), {
        {~short(0), ~short(7), 3},
        {3, 6, ~short(4), ~short(5)}
    });

    // TODO: generate a large testcase using GeoGebra and Mathematica.
}

TEST_CASE( 
        "Initialize and run a UnionOfTwoHalfPlanesRegion", 
        "[VanillaLineContext][UnionOfTwoHalfPlanesRegion]" ) {

    /*

       Start from *

            7         444444 
            70       5    6
           7  0     5     6
           7   03335     6
          7              6 
          7             6
         7              6
         7             6
        7      2222    6
        7     5    0  6
       7     5      0 6
       *11111        0

     */

    std::vector<Coord> verts = {
        Coord(-2, -1), Coord(-1, -1), Coord(0, 0), Coord(1, 0), Coord(2, -1),
        Coord(3, 2), Coord(2, 2), Coord(1, 1), Coord(0, 1), Coord(-1, 2)
    };

    INFO("Verticies received");
    CAPTURE(verts);

    std::vector<Line> lines;
    for (std::size_t i = 0; i < verts.size(); i++) {
        const Coord &p = verts[i];
        const Coord &q = verts[(i + 1) % verts.size()];
        lines.push_back(Line(p, q));
    }

    INFO("Lines generated");
    CAPTURE(lines);

    VanillaLineContext ctx(lines);
    std::vector<LineId> line_ids(lines.size());
    for (std::size_t i = 0; i < lines.size(); i++) {
        const Line &l = lines[i];
        int id = 0;
        while (id < ctx.num_lines() && ctx.line(id) != l)
            id++;
        REQUIRE (id != ctx.num_lines());

        const Coord &p = verts[i];
        const Coord &q = verts[(i + 1) % verts.size()];
        // As the line ids should have signs,
        if (p.x < q.x)
            line_ids[i] = id;
        else
            line_ids[i] = ~id;
    }
    INFO("Converting lines to id's");
    CAPTURE(line_ids);
    std::vector<LineId> expected_line_ids = { 
        1, 5, 2, 0, 6, 
        ~short(4), ~short(5), ~short(3), ~short(0), ~short(7)
    };
    REQUIRE(line_ids == expected_line_ids);

    INFO("Setup done");

    INFO("Initialize U2H");

    /*

       Start from *

            7         444444 
            70       5    6
           7  0     5     6
           7   03335     6
          7              6 
          7             6
         7              6
         7             6
        7      2222    6
        7     5    0  6
       7     5      0 6
       *11111        0

     */

    UnionOfTwoHalfPlanesRegion r50(ctx, 5, 0);
    check_eq(r50.intersection(line_ids), {
        {1, 5, 0, 6, ~4, ~5, ~3, ~0, ~7}
    });
    UnionOfTwoHalfPlanesRegion r3n5(ctx, 3, ~5);
    check_eq(r3n5.intersection(line_ids), {
        {~0, ~7, 3},
        {2, 0, 6, ~4, ~5}
    });
    UnionOfTwoHalfPlanesRegion rn03(ctx, ~0, 3);
    check_eq(rn03.intersection(line_ids), {
        {3, 6, ~4, ~5},
        {~0, ~7, 1, 5, 2}
    });
}

}; // namespace geometry
}; // namespace sofa_designer
