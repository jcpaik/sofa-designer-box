#ifndef SOFA_LINE_CONTEXT_HPP
#define SOFA_LINE_CONTEXT_HPP

#include <cassert>
#include <vector>
#include <map>
#include <tuple>
#include <iostream>

#include <gmp.h>
#include <gmpxx.h>

#include "line.hpp"
#include "line_context.hpp"

namespace sofa_designer {
namespace sofa {

using geometry::Coord;
using geometry::Line;
using geometry::LineId;
using geometry::SlopeId;
using geometry::LineContext;
using geometry::LineArrangement;
using geometry::kV;
using geometry::kU;

// Represents pairs of bands for branch-and-bound of sofas
// The lines should be in the order of:
//
// int_ou
// int_ol
//
//
//
// int_iu
// int_il

struct BandPair {
    Line il, iu, ol, ou;

    BandPair(
            mpq_class slope, 
            mpq_class int_il,
            mpq_class int_iu,
            mpq_class int_ol,
            mpq_class int_ou) :
        il(slope, int_il),
        iu(slope, int_iu),
        ol(slope, int_ol),
        ou(slope, int_ou)
    {

    }

    BandPair(
            Coord unit, 
            mpq_class dot_il,
            mpq_class dot_iu,
            mpq_class dot_ol,
            mpq_class dot_ou) :
        il(unit, dot_il),
        iu(unit, dot_iu),
        ol(unit, dot_ol),
        ou(unit, dot_ou)
    {

    }
};

typedef short BandId;


enum BranchDirection {kDown, kUp};

class SofaLineContext : public LineContext {
    public:
        // does sanity check for given `band_pairs`
        SofaLineContext(
                const std::vector<BandPair> &band_pairs);
        SofaLineContext(
                const SofaLineContext &other,
                SlopeId branch_slope,
                BranchDirection branch_direction);

        SofaLineContext() = delete;
        SofaLineContext(const SofaLineContext &other) = default;
        SofaLineContext(SofaLineContext &&other) = default;
        ~SofaLineContext();
        SofaLineContext &operator=(const SofaLineContext &other) = default;
        SofaLineContext &operator=(SofaLineContext &&other) = default;

        std::size_t num_lines() const
        {
            return lines.size();
        }
        Line line(LineId id) const
        {
            return lines[id];
        }
        std::vector<Line> all_lines() const
        {
            return lines;
        }
        Coord intersection(
                LineId id0, LineId id1) const
        {
            if (id0 < 0)
                id0 = ~id0;
            if (id1 < 0)
                id1 = ~id1;
            if (id0 < id1)
                return intersections[make_l2(id0, id1)];
            else
                return intersections[make_l2(id1, id0)];
        }
        SlopeId slope_id(LineId id) const {return id/4;}

        LineArrangement arrangement(
                LineId id0, 
                LineId id1, 
                LineId id2);

    private:
        // making it const short makes the class non-movable.
        const static short kNoBranch = -1;
        short branched_slope;

        std::size_t n;
        std::vector<Line> lines;
        std::vector<Coord> intersections;
       
        // for any triple of band, store the info of whether they have fixed arrangement
        std::vector<bool> b3_to_determine;
        // if they have same shape, set it to true
        std::vector<bool> b3_determined;
        // whether l3_arr_mem is correct
        std::vector<bool> l3_arr_known;
        // stores partial info of arrangement
        std::vector<bool> l3_arr_mem; 

        inline static LineId l_il(SlopeId s) { return 4*s; }
        inline static LineId l_iu(SlopeId s) { return 4*s+1; }
        inline static LineId l_ol(SlopeId s) { return 4*s+2; }
        inline static LineId l_ou(SlopeId s) { return 4*s+3; }

        inline static BandId l_to_b(LineId l) { return l/2; }

        inline static std::size_t comb2(std::size_t n) { return n*(n-1)/2; }
        inline static std::size_t comb3(std::size_t n) { return n*(n-1)*(n-2)/6; }
        inline static std::size_t num_b3(std::size_t n) { return 8*comb3(n); }
        inline static std::size_t num_l(std::size_t n) { return 4*n; }
        inline static std::size_t num_l2(std::size_t n) { return 16*comb2(n); }
        inline static std::size_t num_l3(std::size_t n) { return 64*comb3(n); }
        inline static std::size_t l3_to_b3(std::size_t l3) { return l3/8; }

        inline thread_local static std::size_t make_b3(BandId id0, BandId id1, BandId id2)
        {
            thread_local static std::vector<std::size_t> ii0(0), ii1(0), ii2(0);
            if (ii0.size() <= id2) {
                std::size_t mval = ii0.size();
                ii0.resize(id2 + 1);
                ii1.resize(id2 + 1);
                ii2.resize(id2 + 1);
                for (std::size_t i = mval; i <= id2; i++) {
                    ii0[i] = 8*(i/2)+i%2;
                    ii1[i] = 8*comb2(i/2)+i%2*2;
                    ii2[i] = 8*comb3(i/2)+i%2*4;
                }
            }
            assert(ii0[id0] + ii1[id1] + ii2[id2] >= 0);
            assert(ii0[id0] + ii1[id1] + ii2[id2] < num_b3(id2 + 1));
            return ii0[id0] + ii1[id1] + ii2[id2];
        }
        inline thread_local static std::size_t make_l2(LineId id0, LineId id1)
        {
            assert(0 <= id0);
            assert(id0 < id1);
            thread_local static std::vector<std::size_t> ii0(0), ii1(0);
            if (ii0.size() <= id1) {
                std::size_t mval = ii0.size();
                ii0.resize(id1 + 1);
                ii1.resize(id1 + 1);
                for (std::size_t i = mval; i <= id1; i++) {
                    ii0[i] = 16*(i/4)+i%4;
                    ii1[i] = 16*comb2(i/4)+i%4*4;
                }
            }
            return ii0[id0] + ii1[id1];
        }
        // assume id0 < id1 < id2;
        inline thread_local static std::size_t make_l3(LineId id0, LineId id1, LineId id2) 
        {
            assert(0 <= id0);
            assert(id0 < id1);
            assert(id1 < id2);
            thread_local static std::vector<std::size_t> ii0(0), ii1(0), ii2(0);
            if (ii0.size() <= id2) {
                std::size_t mval = ii0.size();
                ii0.resize(id2 + 1);
                ii1.resize(id2 + 1);
                ii2.resize(id2 + 1);
                for (std::size_t i = mval; i <= id2; i++) {
                    ii0[i] = 64*(i/4)+(i&2)/2*8+(i&1);
                    ii1[i] = 64*comb2(i/4)+(i&2)/2*16+(i&1)*2;
                    ii2[i] = 64*comb3(i/4)+(i&2)/2*32+(i&1)*4;
                }
            }
            assert(ii0[id0] + ii1[id1] + ii2[id2] >= 0);
            assert(ii0[id0] + ii1[id1] + ii2[id2] < num_l3(id2 + 1));
            return ii0[id0] + ii1[id1] + ii2[id2];
        }

        LineArrangement arrangement_explicit(
                LineId id0, 
                LineId id1, 
                LineId id2)
        {
            if (lines[id1].parallel_intercept(intersection(id0, id2)) >= // the sign
                    lines[id1].intercept)
                return kV;
            else
                return kU;
        }

        Coord intersection_explicit(
                LineId id0, LineId id1)
        {
            return lines[id0].intersection(lines[id1]);
        }

        Line upper(BandId bid) {
            Line l(lines[2*bid]);
            l.intercept = 2*lines[2*bid+1].intercept - lines[2*bid].intercept;
            return l;
        };
        Line lower(BandId bid) {
            Line l(lines[2*bid]);
            l.intercept = 2*lines[2*bid].intercept - lines[2*bid+1].intercept;
            return l;
        };

        void determine_b3_kV(short bid0, short bid1, short bid2, short b3) 
        {
            if (geometry::arrangement_general(lower(bid0), upper(bid1), lower(bid2)) == kV) {
                b3_determined[b3] = true;
                for (short i = 8*b3; i < 8*(b3+1); i++) {
                    l3_arr_known[i] = true;
                    l3_arr_mem[i] = kV;
                }
            } else {
            }
        }

        void determine_b3_kU(short bid0, short bid1, short bid2, short b3) 
        {
            if (geometry::arrangement_general(upper(bid0), lower(bid1), upper(bid2)) == kU) {
                b3_determined[b3] = true;
                for (short i = 8*b3; i < 8*(b3+1); i++) {
                    l3_arr_known[i] = true;
                    l3_arr_mem[i] = kU;
                }
            } else {
            }
        }

        void upd_l3(short id0, short id1, short id2, short l3);
        thread_local static const std::vector<short> &l3_with_l(std::size_t n, LineId l) {

            thread_local static std::map<
                std::tuple<std::size_t, LineId>, 
                std::vector<short> 
                    > mem;

            if (mem.count(std::make_pair(n, l))) {
                return mem[std::make_pair(n, l)];
            } else {
                auto &l3s = mem[std::make_pair(n, l)];
                l3s.clear();
                auto s = l/4;
                for (LineId id0 = 0; id0 < num_l(n); id0++)
                    if (id0/4 != s)
                        for (LineId id1 = id0 + 1; id1 < num_l(n); id1++)
                            if (id1/4 != s && id1/4 != id0/4)
                            {
                                if (l < id0)
                                    l3s.push_back(make_l3(l, id0, id1));
                                else if (l < id1)
                                    l3s.push_back(make_l3(id0, l, id1));
                                else
                                    l3s.push_back(make_l3(id0, id1, l));
                            }
                return l3s;
            }
        };
};

};
};

#endif // SOFA_LINE_CONTEXT_HPP
