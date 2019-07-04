#include "catch.hpp"

#include <algorithm>
#include <random>
#include <vector>
#include <iostream>

#include <gmp.h>
#include <gmpxx.h>

#include "line.hpp"
#include "sofa_line_context.hpp"

namespace sofa_designer {
namespace sofa {

void test_ctx(SofaLineContext &ctx, double fraction = 1.0)
{
    auto lines = ctx.all_lines();

    std::vector< std::tuple<LineId, LineId, LineId> > l3s;
    for (LineId l0 = 0; l0 < ctx.num_lines(); l0++)
        for (LineId l1 = l0 + 1; l1 < ctx.num_lines(); l1++)
            if (ctx.slope_id(l0) != ctx.slope_id(l1))
                for (LineId l2 = l1 + 2; l2 < ctx.num_lines(); l2++)
                    if (ctx.slope_id(l1) != ctx.slope_id(l2))
                        l3s.emplace_back(l0, l1, l2);

    std::random_shuffle(l3s.begin(), l3s.end());
    l3s.resize(l3s.size() * fraction);

    using geometry::Line;
    using geometry::arrangement;

    for (auto t : l3s) {
        LineId l0, l1, l2;
        std::tie(l0, l1, l2) = t;
        auto ans = arrangement(lines[l0], lines[l1], lines[l2]);
        REQUIRE(ctx.arrangement(l0, l1, l2) == ans);
    }
}

TEST_CASE( "Basic functionality of SofaLineContext", "[SofaLineContext]" ) {
    std::vector<BandPair> bps = {
        BandPair(-1, -1, 0, 1, 2),
        BandPair(0, -1, 0, 1, 2),
        BandPair(1, -1, 0, 1, 2),
    };

    SofaLineContext ctx(bps);
    REQUIRE(ctx.all_lines() == std::vector<Line>({
            Line(-1, -1), 
            Line(-1, 0), 
            Line(-1, 1), 
            Line(-1, 2), 
            Line(0, -1), 
            Line(0, 0), 
            Line(0, 1), 
            Line(0, 2), 
            Line(1, -1), 
            Line(1, 0), 
            Line(1, 1), 
            Line(1, 2), 
            }));
    test_ctx(ctx, 0.2);
    SofaLineContext ctx2(ctx, 0, BranchDirection::kDown);
    REQUIRE(ctx2.all_lines() == std::vector<Line>({
            Line(-1, -1), 
            Line(-1, mpq_class("-1/2")), 
            Line(-1, mpq_class("1/2")), 
            Line(-1, 1), 
            Line(0, -1), 
            Line(0, 0), 
            Line(0, 1), 
            Line(0, 2), 
            Line(1, -1), 
            Line(1, 0), 
            Line(1, 1), 
            Line(1, 2), 
            }));
    test_ctx(ctx2, 0.2);
    SofaLineContext ctx3(ctx2, 1, BranchDirection::kUp);
    REQUIRE(ctx3.all_lines() == std::vector<Line>({
            Line(-1, -1), 
            Line(-1, mpq_class("-1/2")), 
            Line(-1, mpq_class("1/2")), 
            Line(-1, 1), 
            Line(0, 0), 
            Line(0, mpq_class("1/2")), 
            Line(0, mpq_class("3/2")), 
            Line(0, 2), 
            Line(1, -1), 
            Line(1, 0), 
            Line(1, 1), 
            Line(1, 2), 
            }));
    test_ctx(ctx3, 0.2);
    SofaLineContext ctx4(ctx3, 2, BranchDirection::kUp);
    REQUIRE(ctx4.all_lines() == std::vector<Line>({
            Line(-1, -1), 
            Line(-1, mpq_class("-1/2")), 
            Line(-1, mpq_class("1/2")), 
            Line(-1, 1), 
            Line(0, 0), 
            Line(0, mpq_class("1/2")), 
            Line(0, mpq_class("3/2")), 
            Line(0, 2), 
            Line(1, 0), 
            Line(1, mpq_class("1/2")), 
            Line(1, mpq_class("3/2")),  
            Line(1, 2), 
            }));
    test_ctx(ctx4, 0.2);
}

TEST_CASE( "Automated stress test on SofaLineContext", "[SofaLineContext]" ) {
    std::vector<mpq_class> slopes;
    mpq_class s_min = -5_mpq, s_max = +6_mpq;
    std::size_t s_n = 10;
    slopes.resize(s_n + 1);
    for (std::size_t i = 0; i <= s_n; i++)
        slopes[i] = (1_mpq-i/s_n) * s_min + (mpq_class(i)/s_n) * s_max;

    std::vector<BandPair> bps;
    std::vector<Line> ls;
    for (auto slope : slopes) {
        CAPTURE(slope);
        bps.emplace_back(slope, -1, 0, 1, 2);
        ls.emplace_back(slope, -1);
        ls.emplace_back(slope, 0);
        ls.emplace_back(slope, 1);
        ls.emplace_back(slope, 2);
    }

    SofaLineContext ctx(bps);
    REQUIRE(ctx.all_lines() == ls);
    for (std::size_t i = 0; i < 100; i++) {
        test_ctx(ctx, 0.02);
        ctx = SofaLineContext(ctx, i % (s_n + 1), i % 2 ? kUp : kDown);
    }
}

}; // namespace geometry
}; // namespace sofa_designer
