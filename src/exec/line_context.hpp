#ifndef LINE_CONTEXT_HPP
#define LINE_CONTEXT_HPP

#include <vector>

#include "line.hpp"

namespace sofa_designer {
namespace geometry {

// maybe TODO: move typedefs inside LineContext
typedef short LineId;
typedef short SlopeId;

// important TODO: specify whether we use negative LineId 
// for member ftns of ctx

class LineContext {
    public:
        // For parallel lines, the value should be the same
        virtual std::size_t num_lines() const = 0;
        virtual Line line(LineId id) const = 0;
        // in the increasing order of LineId from 0 to num_lines() - 1
        virtual std::vector<Line> all_lines() const = 0;
        virtual Coord intersection(
                LineId id0, LineId id1) const = 0;
        virtual SlopeId slope_id(LineId id) const = 0;
        virtual LineArrangement arrangement(
                LineId id0, 
                LineId id1, 
                LineId id2) = 0;

        virtual ~LineContext() = default; 
};

class VanillaLineContext : public LineContext {
    public:
        VanillaLineContext();
        VanillaLineContext(const std::vector<Line> &lines);
        ~VanillaLineContext() = default;

        std::size_t num_lines() const;
        Line line(LineId id) const;
        std::vector<Line> all_lines() const;
        Coord intersection(
                LineId id0, LineId id1) const;
        SlopeId slope_id(LineId id) const;
        LineArrangement arrangement(
                LineId id0, 
                LineId id1, 
                LineId id2);

    private:
        std::vector<Line> lines;
        std::vector<SlopeId> slope_ids;
};

}; // namespace geometry
}; // namespace sofa_designer

#endif /* LINE_CONTEXT_HPP */
