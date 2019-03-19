#ifndef REGION_HPP
#define REGION_HPP

#include <vector>
#include <iostream>

#include "coord.hpp"
#include "line.hpp"
#include "line_context.hpp"

namespace sofa_designer {
namespace geometry {

typedef std::vector<LineId> Polygon;
typedef std::vector<Polygon> Polygons;

class Region {
    public:
        const LineContext &ctx;

        Region() = delete;
        Region &operator=(const Region &other) = delete;
        Region &operator=(Region &&other) = delete;

        Region(const LineContext &ctx) : ctx(ctx) {}
        virtual ~Region() = default;

        virtual Polygons intersection(const Polygon &poly) const = 0;
        Polygons intersection(const Polygons &polys) const;
};

// The structure basically works as a wrapper around one LineId
// The class represents a half-plane with supplied boundary
// The boundary can either have nonnegative id or negative id
class HalfPlaneRegion : public Region {
    public:
        using Region::intersection;
        LineId boundary_id;

        HalfPlaneRegion(
                const LineContext &ctx, 
                LineId boundary_id) : 
            Region(ctx), boundary_id(boundary_id) {}
        ~HalfPlaneRegion() = default;
        bool contains_intersection(LineId l0, LineId l1) const;

        Polygons intersection(const Polygon &poly) const;

    private:
        struct Polyline {
            // the position of LineId's in given polygon
            const LineId *begin, *end;
            // the values of LineId's
            LineId begin_value, end_value;
            bool visited;
            Polyline *nxt_polyline;
        };

        std::size_t build_polylines(
                const Polygon &poly,
                Polyline *polylines) const;
        void link_polylines(
                Polyline *polylines,
                std::size_t num_polylines) const;
        Polygons make_polygons(
                const Polygon &poly,
                Polyline *polylines,
                std::size_t num_polylines) const;

        // used to compare two 
        bool comp_line_out(LineId id0, LineId id1) const {
            return HalfPlaneRegion(ctx, id0).contains_intersection(boundary_id, id1);
        }
};

// Better honest and clear and long when it comes to class names
class UnionOfTwoHalfPlanesRegion : public Region {
    public:
        using Region::intersection;
        LineId bd0, bd1;

        UnionOfTwoHalfPlanesRegion(
                const LineContext &ctx,
                LineId bd0, LineId bd1);
        ~UnionOfTwoHalfPlanesRegion() = default;

        bool intersection_in_h0(LineId l0, LineId l1) const
        {
            return HalfPlaneRegion(ctx, bd0).contains_intersection(l0, l1);
        }
        bool intersection_in_h1(LineId l0, LineId l1) const
        {
            return HalfPlaneRegion(ctx, bd1).contains_intersection(l0, l1);
        }

        Polygons intersection(const Polygon &poly) const;

    private:
        enum BoundaryType {kH0, kH1};

        struct Polyline {
            // the position of LineId's in given polygon
            const LineId *begin, *end;
            // the values of LineId's
            LineId begin_value, end_value;
            BoundaryType begin_type, end_type;
            bool visited;
            Polyline *nxt_polyline;
        };

        std::size_t build_polylines(
                const Polygon &poly,
                Polyline *polylines) const;
        void link_polylines(
                Polyline *polylines,
                std::size_t num_polylines) const;
        Polygons make_polygons(
                const Polygon &poly,
                Polyline *polylines,
                std::size_t num_polylines) const;

        // used to compare two 
        bool comp_line_out_bd0(LineId id0, LineId id1) const {
            return HalfPlaneRegion(ctx, id0).contains_intersection(bd0, id1);
        }
        bool comp_line_out_bd1(LineId id0, LineId id1) const {
            return HalfPlaneRegion(ctx, id0).contains_intersection(bd1, id1);
        }
};

// what to do when connecting?
// read next polyline
//

}; // namespace geometry
}; // namespace sofa_designer

#endif // REGION_HPP
