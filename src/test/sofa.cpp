#include "catch.hpp"

#include <algorithm>
#include <random>
#include <vector>
#include <iostream>

#include <gmp.h>
#include <gmpxx.h>

#include "sofa.hpp"

namespace sofa_designer {
namespace sofa {

TEST_CASE( "Basic functionality of Sofa", "[Sofa]" ) {
    std::vector<mpq_class> x = 
    {
        24_mpq/25_mpz,56_mpq/65_mpz,120_mpq/169_mpz,33_mpq/65_mpz,7_mpq/25_mpz
    };
    std::vector<mpq_class> y = 
    {
        7_mpq/25_mpz,33_mpq/65_mpz,119_mpq/169_mpz,56_mpq/65_mpz,24_mpq/25_mpz
    };
    std::vector<Coord> normals(x.size());
    for (std::size_t i = 0; i < x.size(); i++)
        normals[i] = Coord(x[i], y[i]);
    std::vector<Interval> mu_range = {
        {-84_mpq/125_mpz, 0_mpq},
        {-26_mpq/75_mpz, 0_mpq},
        {0_mpq, 0_mpq}, // fixed
        {0_mpq, 931_mpq/2600_mpz},
        {0_mpq, 2047_mpq/3000_mpz}
    };
    std::vector<Interval> nu_range = {
        {57122_mpq/151725_mpq,62833_mpq/50575_mpz},
        {58334_mpq/70805_mpz,77253_mpq/70805_mpz},
        {338_mpq/357_mpz,169_mpq/119_mpz},
        {314533_mpq/394485_mpz,17576_mpq/10115_mpz},
        {513383_mpq/354025_mpz,685464_mpq/354025_mpz}
    };
    std::size_t mu_fix_idx = 2;
    Sofa s(normals, mu_range, nu_range, mu_fix_idx);
    CAPTURE(s.polygons);
    CAPTURE(s.coord_polygons());
    std::vector< std::vector<Coord> > coord_ans = { 
            { 
            {-1039489_mpq/339864, 0}, 
            {-758342_mpq/467313, 0}, 
            {-3348722_mpq/2251599, 419523_mpq/5253731}, 
            {-724776_mpq/520625, 30199_mpq/74375}, 
            {-1137513_mpq/1047914, 379171_mpq/1197616}, 
            {-517586_mpq/552279, 625_mpq/1547}, 
            {-2_mpq/3, 80_mpq/119}, 
            {0, 0},
            {25_mpq/24, 0}, 
            {37_mpq/40, 2_mpq/5}, 
            {83_mpq/104, 8_mpq/13}, 
            {5_mpq/12, 1}, 
            {-377246_mpq/155771, 1}, 
            {-448941_mpq/184093, 12253_mpq/12376}, 
            {-39832_mpq/14161, 13_mpq/21}, {-1666397_mpq/566440, 2_mpq/5} 
            }
            };
    REQUIRE(s.coord_polygons() == coord_ans);
    Sofa s2(s, 3, HalveType::kNuUp);
    CAPTURE(s2.polygons);
    CAPTURE(s2.coord_polygons());
    REQUIRE(s2.area + s.halve_gain(3, kNuUp) == s.area);
    Sofa s3(s2, 1, HalveType::kMuDown);
    REQUIRE(s3.area + s2.halve_gain(1, kMuDown) == s2.area);
}

TEST_CASE( "Initial sofa list", "[Sofa]" ) {
    std::vector<mpq_class> x = 
    {
        24_mpq/25_mpz,56_mpq/65_mpz,120_mpq/169_mpz,33_mpq/65_mpz,7_mpq/25_mpz
    };
    std::vector<mpq_class> y = 
    {
        7_mpq/25_mpz,33_mpq/65_mpz,119_mpq/169_mpz,56_mpq/65_mpz,24_mpq/25_mpz
    };
    std::vector<Coord> normals(x.size());
    for (std::size_t i = 0; i < x.size(); i++)
        normals[i] = Coord(x[i], y[i]);
    auto sofas = Sofa::a_priori_sofas(
            normals, 2, 3);
    CAPTURE(sofas[1]->coord_polygons());
    // REQUIRE(false);
    for (auto p : sofas)
        delete p;
}

}; // namespace geometry
}; // namespace sofa_designer
