#include "line_context.hpp"

#include "line.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace sofa_designer {
namespace geometry {

template <typename T>
std::vector<T> sort_unique(std::vector<T> arr)
{
    std::sort(arr.begin(), arr.end());
    arr.erase(std::unique(arr.begin(), arr.end()), arr.end());
    return arr;
}

VanillaLineContext::VanillaLineContext()
{

}

VanillaLineContext::VanillaLineContext(
        const std::vector<Line> &l_in) :
    lines(sort_unique(l_in)), slope_ids(lines.size())
{
    if (!lines.size())
        return;

    slope_ids[0] = 0;
    for (std::size_t i = 1; i < lines.size(); i++) {
        if (lines[i - 1].slope != lines[i].slope)
            slope_ids[i] = slope_ids[i - 1] + 1;
        else
            slope_ids[i] = slope_ids[i - 1];
    }
}

std::size_t VanillaLineContext::num_lines() const
{
    return lines.size();
}

Line VanillaLineContext::line(LineId id) const
{
    return lines[id];
}

Coord VanillaLineContext::intersection(LineId id0, LineId id1) const
{
    return sofa_designer::geometry::intersection(line(id0), line(id1));
};

std::vector<Line> VanillaLineContext::all_lines() const
{
    return lines;
}

SlopeId VanillaLineContext::slope_id(LineId id) const
{
    return slope_ids[id];
}

LineArrangement VanillaLineContext::arrangement(
        LineId id0, LineId id1, LineId id2)
{
    return sofa_designer::geometry::arrangement(
            lines[id0], lines[id1], lines[id2]);
}

}; // namespace geometry
}; // namespace sofa_designer
