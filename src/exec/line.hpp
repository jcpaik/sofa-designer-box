#ifndef LINE_HPP
#define LINE_HPP

#include <ostream>

#include <gmpxx.h>

#include "coord.hpp"

namespace sofa_designer {
namespace geometry {

///////////////////////////////////////////////////////////////////////////////
// Declaration
///////////////////////////////////////////////////////////////////////////////

struct Line {
    mpq_class slope, intercept;

    Line();
    Line(const Line &other);
    Line(Line &&other);
    Line(
            const mpq_class &slope,
            const mpq_class &intercept);
    Line(
            const Coord &p0, 
            const Coord &p1);
    Line(
            const Coord &normal_vector, 
            const mpq_class &dot_value);

    Line &operator=(const Line &other);
    Line &operator=(Line &&other);

    // Line is lexicographically ordered by
    // (slope, -intercept)
    bool operator> (const Line &other) const;
    bool operator>=(const Line &other) const;
    bool operator==(const Line &other) const;
    bool operator< (const Line &other) const;
    bool operator<=(const Line &other) const;
    bool operator!=(const Line &other) const;

    Coord intersection(const Line &other) const;

    // Returns the intercept of line parallel to (*this)
    // and passing through p
    mpq_class parallel_intercept(const Coord &p) const;
};

Coord intersection(const Line &l0, const Line &l1);

typedef bool LineArrangement;
const LineArrangement kU = false;
const LineArrangement kV = true;

LineArrangement arrangement(
        Line l0, Line l1, Line l2);

std::ostream &operator<<(
        std::ostream &out, const Line &line);

///////////////////////////////////////////////////////////////////////////////
// Definition
///////////////////////////////////////////////////////////////////////////////

inline Line::Line() : 
    slope(0L), intercept(0L)
{

}

inline Line::Line(const Line &other) : 
    slope(other.slope), intercept(other.intercept)
{

}

inline Line::Line(Line &&other) :
    slope(std::move(other.slope)),
    intercept(std::move(other.intercept))
{

}

inline Line::Line(
        const mpq_class &slope,
        const mpq_class &intercept) :
    slope(slope), intercept(intercept)
{

}

inline Line::Line(
        const Coord &p0,
        const Coord &p1) :
    slope((p1.y - p0.y) / (p1.x - p0.x)),
    intercept(
            (p1.x * p0.y - p0.x * p1.y) / 
            (p1.x - p0.x))
{

}

inline Line::Line(
        const Coord &n,        // normal_vector
        const mpq_class &d) :  // dot_value
    slope(-n.x / n.y),
    intercept(d / n.y)
{

}

inline Line &Line::operator=(const Line &other)
{
    this->slope = other.slope;
    this->intercept = other.intercept;
    return (*this);
}

inline Line &Line::operator=(Line &&other)
{
    this->slope = std::move(other.slope);
    this->intercept = std::move(other.intercept);
    return (*this);
}

// minor TODO: change all comparsions to use cmp
inline bool Line::operator>(const Line &other) const
{
    if (this->slope > other.slope)
        return true;
    else if (this->slope == other.slope)
        return this->intercept > other.intercept;
    else
        return false;
}

inline bool Line::operator>=(const Line &other) const
{
    if (this->slope >= other.slope)
        return this->intercept >= other.intercept;
    else
        return false;
}

inline bool Line::operator==(const Line &other) const
{
    return
        (this->slope == other.slope) &&
        (this->intercept == other.intercept);
}

inline bool Line::operator<(const Line &other) const
{
    if (this->slope < other.slope)
        return true;
    else if (this->slope == other.slope)
        return this->intercept < other.intercept;
    else
        return false;
}

inline bool Line::operator<=(const Line &other) const
{
    if (this->slope <= other.slope)
        return this->intercept <= other.intercept;
    else
        return false;
}

inline bool Line::operator!=(const Line &other) const
{
    if (this->slope == other.slope)
        return this->intercept != other.intercept;
    else
        return true;
}

inline Coord Line::intersection(const Line &other) const
{
    // Hope compiler will optimize this
    const mpq_class &ts = this->slope, &ti = this->intercept;
    const mpq_class &os = other.slope, &oi = other.intercept;
    mpq_class ds = os - ts;

    return Coord (
        (ti - oi) / ds, // x
        (os*ti - oi*ts) / std::move(ds) // y
    );
}

// TODO: test this function
inline mpq_class Line::parallel_intercept(const Coord &p) const
{
    return p.y - this->slope * p.x;
}

inline Coord intersection(const Line &l0, const Line &l1)
{
    return l0.intersection(l1);
}

// Assuming slopes are in strictly increasing order
inline LineArrangement arrangement_general(
        const Line &l0, const Line &l1, const Line &l2)
{
    Coord p = intersection(l0, l2);
    if (l1.intercept > l1.parallel_intercept(p))
        return kU;
    else
        return kV;
}

// Assuming lines are monotonically increasing
inline LineArrangement arrangement_ordered(
        const Line &l0, const Line &l1, const Line &l2)
{
    if (l0.slope == l1.slope)
        return kV;
    else if (l1.slope == l2.slope)
        return kV;
    else
        return arrangement_general(l0, l1, l2);
}

// maybe TODO: change to const ref for avoiding three copies
inline LineArrangement arrangement(
        Line l0, Line l1, Line l2)
{
    if (l0 > l1)
        std::swap(l0, l1);
    if (l1 > l2)
        std::swap(l1, l2);
    if (l0 > l1)
        std::swap(l0, l1);
    return arrangement_ordered(l0, l1, l2);
}

inline std::ostream &operator<<(
        std::ostream &out, const Line &line)
{
    out << 
        "y = " << line.slope <<
        "*x + " << line.intercept;
    return out;
}


}; // namespace geometry
}; // namespace sofa_designer

#endif // LINE_HPP
