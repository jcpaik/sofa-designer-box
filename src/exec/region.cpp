#include "region.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace sofa_designer {
namespace geometry { 

Polygons Region::intersection(const Polygons &polys) const
{
    Polygons res;
    for (Polygon p : polys) {
        Polygons to_add = intersection(p);
        res.insert(res.end(), to_add.begin(), to_add.end());
    }
    return res;
}

bool HalfPlaneRegion::contains_intersection(LineId l0, LineId l1) const
{
    if (l0 < 0)
        l0 = ~l0;
    if (l1 < 0)
        l1 = ~l1;
    if (l0 > l1)
        std::swap(l0, l1);

    // the supplied lines should intersect trasversly
    assert(ctx.slope_id(l0) != ctx.slope_id(l1));

    // first consider the case where l is nonnegative
    LineId l = (boundary_id >= 0 ? boundary_id : ~boundary_id);

    // exclude when any of two lines are same
    if (l0 == l || l1 == l)
        return false;

    bool below_l;
    if (ctx.slope_id(l0) == ctx.slope_id(l)) {
        // l0 and l parallel
        // the intersection of l0 and l1 is below l iff l is above l0
        below_l = (l0 < l);
    } else if (ctx.slope_id(l1) == ctx.slope_id(l)) {
        // l1 and l parallel
        // the intersection of l0 and l1 is below l iff l is above l1
        below_l = (l1 < l);
    } else if (l < l0) {
        // l < l0 < l1
        below_l = (const_cast<LineContext&>(ctx).arrangement(l, l0, l1) == kV);
    } else if (l1 < l) {
        // l0 < l1 < l
        below_l = (const_cast<LineContext&>(ctx).arrangement(l0, l1, l) == kV);
    } else {
        // l0 < l < l1
        below_l = (const_cast<LineContext&>(ctx).arrangement(l0, l, l1) == kU);
    }

    if (boundary_id >= 0)
        return !below_l;
    else
        return below_l;
}

std::size_t HalfPlaneRegion::build_polylines(
        const Polygon &poly,
        Polyline *polylines) const
{
    LineId l = *poly.rbegin();
    LineId m = *poly.begin();
    bool p_in_region = contains_intersection(l, m);

    polylines[0].begin = nullptr;

    // iterate through
    // maybe TODO: take away .size() call
    Polyline *cur_polyline = polylines;
    for (std::size_t i = 0; i < poly.size(); i++) {
        LineId n = poly[(i + 1) % poly.size()];
        bool q_in_region = contains_intersection(m, n);

        if (!p_in_region && q_in_region) {
            // the line m is entering the region
            cur_polyline->begin = &(poly[i]);
            cur_polyline->begin_value = poly[i];
        } else if (p_in_region && !q_in_region) {
            // the line m is going out of the region
            cur_polyline->end = &(poly[i]);
            cur_polyline->end_value = poly[i];
            cur_polyline->visited = false;
            cur_polyline++;
            cur_polyline->begin = nullptr;
        }

        // pass values used
        p_in_region = q_in_region;
        l = m;
        m = n;
    }

    if (cur_polyline->begin) {
        polylines[0].begin = cur_polyline->begin;
        polylines[0].begin_value = cur_polyline->begin_value;
    }

    return cur_polyline - polylines;
}

void HalfPlaneRegion::link_polylines(
        Polyline *polylines,
        std::size_t num_polylines) const
{
    Polyline *pl_srt_b[num_polylines];
    Polyline *pl_srt_e[num_polylines];
    for (int i = 0; i < num_polylines; i++)
        pl_srt_b[i] = pl_srt_e[i] = polylines + i;

    std::sort(pl_srt_b, pl_srt_b + num_polylines, 
            [this](const Polyline * pl0, const Polyline * pl1){
            // line enters the region, so reverse the line value to make it go out of region
            return comp_line_out(~(pl0->begin_value), ~(pl1->begin_value));
            });
    std::sort(pl_srt_e, pl_srt_e + num_polylines, 
            [this](const Polyline * pl0, const Polyline * pl1){
            // line exits the region, so the line is aligned to the out direction from region
            return comp_line_out(pl0->end_value, pl1->end_value);
            });

    for (int i = 0; i < num_polylines; i++)
        pl_srt_e[i]->nxt_polyline = pl_srt_b[i];

}

Polygons HalfPlaneRegion::make_polygons(
        const Polygon &poly,
        Polyline *polylines,
        std::size_t num_polylines) const
{
        Polygons polygons;

        for (int i = 0; i < num_polylines; i++)
            if (!polylines[i].visited) {
                Polygon cur_polygon;
                Polyline *cur_polyline = polylines + i;
                while (!cur_polyline->visited) {
                    cur_polyline->visited = true;
                    // move the polyline in
                    if (cur_polyline->begin <= cur_polyline->end) {
                        cur_polygon.insert(cur_polygon.end(), 
                                cur_polyline->begin, cur_polyline->end + 1);
                    } else {
                        cur_polygon.insert(cur_polygon.end(), 
                                cur_polyline->begin, &(poly[0]) + poly.size());
                        cur_polygon.insert(cur_polygon.end(), 
                                &(poly[0]), cur_polyline->end + 1);
                    }
                    // consider in-between
                    cur_polygon.push_back(boundary_id);
                    cur_polyline = cur_polyline->nxt_polyline;
                }

                polygons.push_back(std::move(cur_polygon));
            }

        return polygons;
}

Polygons HalfPlaneRegion::intersection(const Polygon &poly) const
{
    if (!poly.size())
        return {};

    assert(poly.size() >= std::size_t(3));

    Polyline polylines[poly.size() / 2 + 1];
    std::size_t num_polylines = build_polylines(poly, polylines);

    if (num_polylines == 0) {
        // nothing has written: neither out or in
        if (contains_intersection(poly[0], poly[1]))
            return {poly};
        else
            return {};
    } else {
        link_polylines(polylines, num_polylines);
        return make_polygons(poly, polylines, num_polylines);
    }
}

UnionOfTwoHalfPlanesRegion::UnionOfTwoHalfPlanesRegion(
        const LineContext &ctx,
        LineId bd0, LineId bd1) :
    Region(ctx), bd0(bd0), bd1(bd1)
{
    bool flip = ((bd0 < 0) != (bd1 < 0));
    // make everything unsigned
    if (bd0 < 0) bd0 = ~bd0;
    if (bd1 < 0) bd1 = ~bd1;
    if (bd0 < bd1) flip = (!flip);
    if (flip) std::swap(this->bd0, this->bd1);
    // std::cout << this->bd0 << " " << this->bd1 << " bd0 bd1" << std::endl;
}

std::size_t UnionOfTwoHalfPlanesRegion::build_polylines(
        const Polygon &poly,
        Polyline *polylines) const
{
    LineId l = *poly.rbegin();
    LineId m = *poly.begin();
    bool p_in_h0 = intersection_in_h0(l, m);
    bool p_in_h1 = intersection_in_h1(l, m);
    bool p_in_region = p_in_h0 || p_in_h1;

    Polyline *cur_polyline = polylines;
    // iterate through
    for (std::size_t i = 0; i < poly.size(); i++) {
        LineId n = poly[(i + 1) % poly.size()];
        bool q_in_h0 = intersection_in_h0(m, n);
        bool q_in_h1 = intersection_in_h1(m, n);
        bool q_in_region = q_in_h0 || q_in_h1;

        if (!p_in_region && q_in_region) {
            // the line m is entering the region
            // std::cout << "enter" << std::endl;
            cur_polyline->begin = &(poly[i]);
            cur_polyline->begin_value = poly[i];
            BoundaryType type;
            if (!p_in_h0 && !q_in_h0)
                type = kH1;
            else if (!p_in_h1 && !q_in_h1)
                type = kH0;
            else {
                assert(m != bd0 && m != bd1 && m != ~bd0 && m != ~bd1);
                if (HalfPlaneRegion(ctx, m).contains_intersection(bd0, bd1))
                    type = kH1;
                else 
                    type = kH0;
            }
            cur_polyline->begin_type = type;
        } else if (p_in_region && !q_in_region) {
            // the line m is going out of the region
            // std::cout << "exit" << std::endl;
            cur_polyline->end = &(poly[i]);
            cur_polyline->end_value = poly[i];
            BoundaryType type;
            if (!p_in_h0 && !q_in_h0)
                type = kH1;
            else if (!p_in_h1 && !q_in_h1)
                type = kH0;
            else {
                assert(m != bd0 && m != bd1 && m != ~bd0 && m != ~bd1);
                if (HalfPlaneRegion(ctx, m).contains_intersection(bd0, bd1))
                    type = kH0;
                else 
                    type = kH1;
            }
            cur_polyline->end_type = type;
            cur_polyline->visited = false;
            cur_polyline++;
            cur_polyline->begin = nullptr;
        } else if (p_in_region && q_in_region && 
                (p_in_h0 != q_in_h0) && (p_in_h1 != q_in_h1)) {
            // the line m might cut pass through corner
            assert(m != bd0 && m != bd1 && m != ~bd0 && m != ~bd1);
            if (p_in_h0) {
                assert(p_in_h0 && !p_in_h1 && !q_in_h0 && q_in_h1);
                if (HalfPlaneRegion(ctx, m).contains_intersection(bd0, bd1)) {
                    // the line goes out and in
                    // going out
                    cur_polyline->end = &(poly[i]);
                    cur_polyline->end_value = poly[i];
                    cur_polyline->end_type = kH0;
                    cur_polyline->visited = false;
                    cur_polyline++;
                    // going in
                    cur_polyline->begin = &(poly[i]);
                    cur_polyline->begin_value = poly[i];
                    cur_polyline->begin_type = kH1;
                }
            } else {
                assert(!p_in_h0 && p_in_h1 && q_in_h0 && !q_in_h1);
                if (!HalfPlaneRegion(ctx, m).contains_intersection(bd0, bd1)) {
                    // the line goes out and in
                    // going out
                    cur_polyline->end = &(poly[i]);
                    cur_polyline->end_value = poly[i];
                    cur_polyline->end_type = kH1;
                    cur_polyline->visited = false;
                    cur_polyline++;
                    // going in
                    cur_polyline->begin = &(poly[i]);
                    cur_polyline->begin_value = poly[i];
                    cur_polyline->begin_type = kH0;
                }
            }
        }

        // pass values used
        p_in_h0 = q_in_h0;
        p_in_h1 = q_in_h1;
        p_in_region = q_in_region;
        l = m;
        m = n;
    }

    // enclose the start
    if (cur_polyline->begin) {
        polylines[0].begin = cur_polyline->begin;
        polylines[0].begin_value = cur_polyline->begin_value;
        polylines[0].begin_type = cur_polyline->begin_type;
    }

    return cur_polyline - polylines;
}

void UnionOfTwoHalfPlanesRegion::link_polylines(
        Polyline *polylines,
        std::size_t num_polylines) const
{
    Polyline *pl_b_h0[num_polylines];
    Polyline *pl_e_h0[num_polylines];
    Polyline *pl_b_h1[num_polylines];
    Polyline *pl_e_h1[num_polylines];
    std::size_t num_b_h0 = 0, num_e_h0 = 0, num_b_h1 = 0, num_e_h1 = 0;
    for (int i = 0; i < num_polylines; i++) {
        Polyline *p = polylines + i;
        if (p->begin_type == kH0)
            pl_b_h0[num_b_h0++] = p;
        else
            pl_b_h1[num_b_h1++] = p;
        if (p->end_type == kH0)
            pl_e_h0[num_e_h0++] = p;
        else
            pl_e_h1[num_e_h1++] = p;
        /*
        std::cout << p->begin_value << " " << p->end_value << std::endl;
        std::cout << p->begin_type << " " << p->end_type << std::endl;
        */
    }

    // polylines entering the region: so reverse direction
    std::sort(pl_b_h0, pl_b_h0 + num_b_h0, 
            [this](const Polyline * pl0, const Polyline * pl1){
            return comp_line_out_bd0(~(pl0->begin_value), ~(pl1->begin_value));
            });
    std::sort(pl_b_h1, pl_b_h1 + num_b_h1, 
            [this](const Polyline * pl0, const Polyline * pl1){
            return comp_line_out_bd1(~(pl0->begin_value), ~(pl1->begin_value));
            });
    // polylines exiting the region, no need to reverse
    std::sort(pl_e_h0, pl_e_h0 + num_e_h0, 
            [this](const Polyline * pl0, const Polyline * pl1){
            return comp_line_out_bd0(pl0->end_value, pl1->end_value);
            });
    std::sort(pl_e_h1, pl_e_h1 + num_e_h1, 
            [this](const Polyline * pl0, const Polyline * pl1){
            return comp_line_out_bd1(pl0->end_value, pl1->end_value);
            });

    if (num_b_h0 == num_e_h0) {
        assert(num_b_h1 == num_e_h1); 
        for (std::size_t i = 0; i < num_b_h0; i++)
            pl_e_h0[i]->nxt_polyline = pl_b_h0[i];
        for (std::size_t i = 0; i < num_b_h1; i++)
            pl_e_h1[i]->nxt_polyline = pl_b_h1[i];
    } else {
        //             .
        //     /\     / \
        //    e  b   e   \
        //  ---->b0>----. \
        //              | /
        //              |b
        //              v
        //              b1
        //              v
        //              |e
        //              | \
        //              | /
        //              |b
        assert(num_b_h0 + 1 == num_e_h0 && num_b_h1 == num_e_h1 + 1);
        // std::cout << num_b_h0 << num_e_h0 << num_b_h1 << num_e_h1 << std::endl;
        for (std::size_t i = 0; i < num_e_h0 - 1; i++)
            pl_e_h0[i]->nxt_polyline = pl_b_h0[i];
        pl_e_h0[num_e_h0 - 1]->nxt_polyline = pl_b_h1[0];
        for (std::size_t i = 0; i < num_e_h1; i++)
            pl_e_h1[i]->nxt_polyline = pl_b_h1[i + 1];
    }
}

Polygons UnionOfTwoHalfPlanesRegion::make_polygons(
        const Polygon &poly,
        Polyline *polylines,
        std::size_t num_polylines) const
{
    /*
    std::cout << polylines << std::endl;
    std::cout << polylines + 1 << std::endl;
    std::cout << polylines->nxt_polyline << std::endl;
    std::cout << (polylines + 1)->nxt_polyline << std::endl;
    */
    Polygons polygons;

    for (int i = 0; i < num_polylines; i++)
        if (!polylines[i].visited) {
            Polygon cur_polygon;
            Polyline *cur_polyline = polylines + i;
            while (!cur_polyline->visited) {
                cur_polyline->visited = true;
                // move the polyline in
                if (cur_polyline->begin < cur_polyline->end) {
                    cur_polygon.insert(cur_polygon.end(), 
                            cur_polyline->begin, cur_polyline->end + 1);
                } else {
                    cur_polygon.insert(cur_polygon.end(), 
                            cur_polyline->begin, &(poly[0]) + poly.size());
                    cur_polygon.insert(cur_polygon.end(), 
                            &(poly[0]), cur_polyline->end + 1);
                }
                auto nxt_polyline = cur_polyline->nxt_polyline;
                // consider in-between
                //             .
                //     /\     / \
                //    e  b   e   \
                //  ---->bd0>---. \
                //              | /
                //              |b
                //              v
                //              bd1
                //              v
                //              |e
                //              | \
                //              | /
                //              |b
                // bring a formal proof of why this is correct later
                if (cur_polyline->end_type == kH0) {
                    if (nxt_polyline->begin_type == kH1) {
                        // need to connect them around a corner
                        if (cur_polyline->end_value != bd1) {
                            if (nxt_polyline->begin_value != bd0) {
                                cur_polygon.push_back(bd0);
                                cur_polygon.push_back(bd1);
                            }
                        } 
                    } else { // nxt_polyline->begin_type == kH0
                        cur_polygon.push_back(bd0);
                    }
                } else { // cur_polyline->end_type == kH1
                    assert(nxt_polyline->begin_type == kH1);
                    cur_polygon.push_back(bd1);
                }

                cur_polyline = cur_polyline->nxt_polyline;
            }
            polygons.push_back(std::move(cur_polygon));
        }

    return polygons;
}

Polygons UnionOfTwoHalfPlanesRegion::intersection(const Polygon &poly) const
{
    if (!poly.size())
        return {};

    assert(poly.size() >= std::size_t(3));
    Polyline polylines[poly.size() / 2 + 1];
    polylines[0].begin = nullptr;

    // std::cout << "build poly" << std::endl;
    std::size_t num_polylines = build_polylines(poly, polylines);

    if (!num_polylines) {
        LineId l = *poly.rbegin();
        LineId m = *poly.begin();
        bool p_in_h0 = intersection_in_h0(l, m);
        bool p_in_h1 = intersection_in_h1(l, m);
        bool p_in_region = p_in_h0 || p_in_h1;
        if (p_in_region)
            return {poly};
        else
            return {};
    } else {
        link_polylines(polylines, num_polylines);
        return make_polygons(poly, polylines, num_polylines);
    }
}

}; // namespace geometry
}; // namespace sofa_designer
