#include "sofa_line_context.hpp"

#include <cassert>
#include <iostream>

namespace sofa_designer {
namespace sofa {

SofaLineContext::SofaLineContext(
        const std::vector<BandPair> &band_pairs) :

    branched_slope(kNoBranch),

    n(band_pairs.size()),
    lines(0),
    intersections(num_l2(n)),

    b3_to_determine (num_b3(n), true ),
    b3_determined   (num_b3(n), false),
    l3_arr_known    (num_l3(n), false), 
    l3_arr_mem      (num_l3(n), false)

{
    // adjacent pairs have increasing slopes
    for (std::size_t i = 0; i < n-1; i++) {
        assert(band_pairs[i].il.slope < band_pairs[i+1].il.slope);
    }

    // each band_pair should have lines in increasing intercept
    for (const auto &bp : band_pairs) {
        assert(bp.il.slope == bp.iu.slope);
        assert(bp.iu.slope == bp.ol.slope);
        assert(bp.ol.slope == bp.ou.slope);
        assert(bp.il.intercept < bp.iu.intercept);
        assert(bp.iu.intercept < bp.ol.intercept);
        assert(bp.ol.intercept < bp.ou.intercept);
        lines.push_back(bp.il);
        lines.push_back(bp.iu);
        lines.push_back(bp.ol);
        lines.push_back(bp.ou);
    }

    // update intersections
    for (std::size_t i = 0; i < num_l(n); i++) {
        for (std::size_t j = i + 1; j < num_l(n); j++) {
            if (slope_id(i) != slope_id(j)) {
                intersections[make_l2(i, j)] = geometry::intersection(lines[i], lines[j]);
            }
        }
    }
}

SofaLineContext::SofaLineContext( 
        const SofaLineContext &other,
        SlopeId bs,
        BranchDirection branch_direction) :

    branched_slope(bs),

    n(other.n),
    lines(other.lines),
    intersections(other.intersections),

    b3_to_determine (other.b3_to_determine),
    b3_determined   (other.b3_determined  ),
    l3_arr_known    (other.l3_arr_known   ), 
    l3_arr_mem      (other.l3_arr_mem     )

{
    mpq_class &l_il_i = lines[l_il(bs)].intercept;
    mpq_class &l_iu_i = lines[l_iu(bs)].intercept;
    mpq_class &l_ol_i = lines[l_ol(bs)].intercept;
    mpq_class &l_ou_i = lines[l_ou(bs)].intercept;
    mpq_class igap = (l_ou_i - l_ol_i) / 2;

    if (branch_direction == kDown) {
        // l_il_i = l_il_i
        l_iu_i -= igap;
        l_ou_i = l_ol_i;
        l_ol_i -= igap;

        // update intersections
        for (LineId l = 0; l < num_l(n); l++) {
            if (slope_id(l) == bs)
                continue;

            short l2_iu, l2_ol, l2_ou;
            if (slope_id(l) < bs) {
                l2_iu = make_l2(l, l_iu(bs));
                l2_ol = make_l2(l, l_ol(bs));
                l2_ou = make_l2(l, l_ou(bs));
            } else if (slope_id(l) > bs) {
                l2_iu = make_l2(l_iu(bs), l);
                l2_ol = make_l2(l_ol(bs), l);
                l2_ou = make_l2(l_ou(bs), l);
            }
            // trnasfer ol to ou
            // how can I move
            intersections[l2_ou] = Coord(intersections[l2_ol]);
            // update iu and ol
            intersections[l2_iu] = intersection_explicit(l, l_iu(bs));
            intersections[l2_ol] = intersection_explicit(l, l_ol(bs));
        }

        // update memory
        const auto &l3_iu = l3_with_l(n, l_iu(bs));
        const auto &l3_ol = l3_with_l(n, l_ol(bs));
        const auto &l3_ou = l3_with_l(n, l_ou(bs));
        for (std::size_t i = 0; i < l3_iu.size(); i++) {
            auto b3 = l3_to_b3(l3_iu[i]);
            // if the band triple is not determined, update
            if (!b3_determined[b3]) {
                // as new band triple is made, mark it to determine
                b3_to_determine[b3] = true;
                // iu and ol changed so invalidate
                l3_arr_known[l3_iu[i]] = false;
                l3_arr_known[l3_ol[i]] = false;
                // transfer ol to ou
                l3_arr_known[l3_ou[i]] = l3_arr_known[l3_ol[i]];
                l3_arr_mem[l3_ou[i]]   = l3_arr_mem[l3_ol[i]];
            }
        }
    } else { // branch_drection == kUp
        l_il_i = l_iu_i;
        l_iu_i += igap;
        l_ol_i += igap;
        // l_ou_i = l_ou_i

        // update intersections
        for (LineId l = 0; l < num_l(n); l++) {
            if (slope_id(l) == bs)
                continue;

            short l2_il, l2_iu, l2_ol;
            if (slope_id(l) < bs) {
                l2_il = make_l2(l, l_il(bs));
                l2_iu = make_l2(l, l_iu(bs));
                l2_ol = make_l2(l, l_ol(bs));
            } else if (slope_id(l) > bs) {
                l2_il = make_l2(l_il(bs), l);
                l2_iu = make_l2(l_iu(bs), l);
                l2_ol = make_l2(l_ol(bs), l);
            }
            // trnasfer iu to il
            intersections[l2_il] = std::move(intersections[l2_iu]);
            // update iu and ol
            intersections[l2_iu] = intersection_explicit(l, l_iu(bs));
            intersections[l2_ol] = intersection_explicit(l, l_ol(bs));
        }

        // update memory
        const auto &l3_il = l3_with_l(n, l_il(bs));
        const auto &l3_iu = l3_with_l(n, l_iu(bs));
        const auto &l3_ol = l3_with_l(n, l_ol(bs));
        for (std::size_t i = 0; i < l3_il.size(); i++) {
            auto b3 = l3_to_b3(l3_il[i]);
            // if the band triple is not determined, update
            if (!b3_determined[b3]) {
                // as new band triple is made, mark it to determine
                b3_to_determine[b3] = true;
                // iu and ol changed so invalidate
                l3_arr_known[l3_iu[i]] = false;
                l3_arr_known[l3_ol[i]] = false;
                // transfer iu to il
                l3_arr_known[l3_il[i]] = l3_arr_known[l3_iu[i]];
                l3_arr_mem[l3_il[i]]   = l3_arr_mem[l3_iu[i]];
            }
        }
    }

}

SofaLineContext::~SofaLineContext()
{

}

LineArrangement SofaLineContext::arrangement(
        LineId id0, LineId id1, LineId id2)
{
    int l3 = make_l3(id0, id1, id2);
    if (l3_arr_known[l3]) {
        return l3_arr_mem[l3];
    } else {
        upd_l3(id0, id1, id2, l3);
        return l3_arr_mem[l3];
    }
}

typedef std::vector<bool>::reference bref;

void SofaLineContext::upd_l3(
        short id0, short id1, short id2, short l3)
{
    short b3 = l3_to_b3(l3);
    if (arrangement_explicit(id0, id1, id2) == kV) {
        l3_arr_mem[l3] = kV;
        if (b3_to_determine[b3]) {
            determine_b3_kV(l_to_b(id0), l_to_b(id1), l_to_b(id2), b3);
            b3_to_determine[b3] = false;
        }
    } else { // arrangement_explicit(id0, id1, id2) == kU
        l3_arr_mem[l3] = kU;
        if (b3_to_determine[b3]) {
            determine_b3_kU(l_to_b(id0), l_to_b(id1), l_to_b(id2), b3);
            b3_to_determine[b3] = false;
        }
    }
}

};
};
