#ifndef SOFA_HPP
#define SOFA_HPP

#include <cassert>
#include <cstdio>
#include <vector>
#include <utility>
#include <tuple>

#include "line_context.hpp"
#include "region.hpp"
#include "sofa_line_context.hpp"

namespace sofa_designer {
namespace sofa {

using namespace sofa_designer::geometry;

struct Interval {
    mpq_class min, max;
    mpq_class avg() const {
        return (min + max) / 2_mpq;
    }
};

enum HalveType {kMuDown, kMuUp, kNuDown, kNuUp};

// these should be sufficient for constructing a sofa data
struct SofaParams {
    std::vector<Interval> mu_range, nu_range;

    void write(FILE *file);
    static SofaParams read(FILE *file);
};

struct SofaMetadata {
    std::vector<Coord> normals;
    std::vector<SofaParams> init_params;
    std::size_t mu_fix_idx;
    // generates a list of 'metadata's for 
    static SofaMetadata a_priori_sofa_metadata(
            std::vector<Coord> normals,
            std::size_t mu_fix_idx,
            std::size_t n);

    void write(FILE *file);
    static SofaMetadata read(FILE *file);
};

class Sofa {
    public:
        static std::vector<Sofa*> a_priori_sofas(
                std::vector<Coord> normals,
                std::size_t mu_fix_idx,
                std::size_t n);
    public: 
        Sofa() = delete;
        ~Sofa() = default;
        std::size_t n;
        std::size_t mu_fix_idx;
        std::vector<Coord> mu, nu;
        std::vector<Interval> mu_range, nu_range;
        SofaLineContext ctx;
        Polygons polygons;
        mpq_class area;

        // normals should contain unit vectors
        // mu_range and nu_range should contain 
        // intervals of positive length
        // except for mu_range[mu_fix_idx]
        Sofa(
                std::vector<Coord> normals,
                std::vector<Interval> mu_range,
                std::vector<Interval> nu_range,
                std::size_t mu_fix_idx);
        static std::vector<Coord> mu_to_nu(std::vector<Coord> mu);
        static std::vector<BandPair> make_band_pairs(
                const std::vector<Coord> &mu,
                const std::vector<Coord> &nu,
                const std::vector<Interval> &mu_range,
                const std::vector<Interval> &nu_range,
                std::size_t mu_fix_idx);

        static BranchDirection halve_dir(HalveType t) {
            switch(t) {
                case kMuDown:
                case kNuDown:
                    return kDown;
                case kMuUp:
                case kNuUp:
                    return kUp;
            }
        }
        static bool is_mu(HalveType t) {
            switch(t) {
                case kMuDown:
                case kMuUp:
                    return true;
                case kNuDown:
                case kNuUp:
                    return false;
            }
        }

        Sofa(
                const Sofa &other, 
                std::size_t idx,
                HalveType t);

        std::vector<Coord> poly_to_coord(Polygon p);
        std::vector< std::vector<Coord> > coord_polygons();
        mpq_class calc_area(Polygon p);
        mpq_class calc_area(Polygons p);

        mpq_class halve_gain(
                std::size_t idx,
                HalveType t);

        LineId hl() {return n*4;}
        LineId hu() {return n*4+3;}
        LineId ldd(std::size_t i) {return (i+n+1)*4;}
        LineId ldu(std::size_t i) {return (i+n+1)*4+1;}
        LineId lud(std::size_t i) {return (i+n+1)*4+2;}
        LineId luu(std::size_t i) {return (i+n+1)*4+3;}
        LineId rdd(std::size_t i) {return (i)*4;}
        LineId rdu(std::size_t i) {return (i)*4+1;}
        LineId rud(std::size_t i) {return (i)*4+2;}
        LineId ruu(std::size_t i) {return (i)*4+3;}
};

};
};

#endif // SOFA_HPP
