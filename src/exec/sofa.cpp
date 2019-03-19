#include "sofa.hpp"

namespace sofa_designer {
namespace sofa {

using sofa_designer::geometry::intersection;

SofaMetadata SofaMetadata::a_priori_sofa_metadata(
        std::vector<Coord> normals,
        std::size_t mu_fix_idx,
        std::size_t n)
{
    std::vector<Coord> mu = normals;
    std::vector<Coord> nu = Sofa::mu_to_nu(normals);
    SofaMetadata md = {
        normals,
        {},
        mu_fix_idx,
    };
    // have to init md.init_params
    // range of nu for mu_fix_idx
    mpq_class main_nu_min = 0;
    mpq_class main_nu_max = 1_mpq / normals[mu_fix_idx].y;
    md.init_params.resize(n);
    Line u_line(0, 1);
    Line l_line(0, 0);
    for (std::size_t i = 0; i < n; i++) {
        mpq_class this_nu_min = main_nu_max / n * i;
        mpq_class this_nu_max = main_nu_max / n * (i + 1);
        Line upperl = Line(nu[mu_fix_idx], this_nu_max);
        Line lowerl = Line(nu[mu_fix_idx], this_nu_min);
        Coord l0 = intersection(upperl, u_line);
        Coord l1 = intersection(lowerl, u_line);
        Coord l2 = intersection(upperl, l_line);
        Coord l3 = intersection(lowerl, l_line);
        Coord r0 = intersection(Line(mu[mu_fix_idx], 0), u_line);
        Coord r1 = intersection(Line(mu[mu_fix_idx], 0), l_line);

        md.init_params[i].mu_range.resize(normals.size());
        md.init_params[i].nu_range.resize(normals.size());
        for (std::size_t j = 0; j < normals.size(); j++) {
            auto &mr = md.init_params[i].mu_range[j];
            auto &nr = md.init_params[i].nu_range[j];
            if (j < mu_fix_idx) {
                mr.min = r0.dot(mu[j]);
                mr.max = r1.dot(mu[j]);
                nr.min = l3.dot(nu[j]);
                nr.max = l0.dot(nu[j]);
            } else if (j == mu_fix_idx) {
                mr.min = 0;
                mr.max = 0;
                nr.min = this_nu_min;
                nr.max = this_nu_max;
            } else if (j > mu_fix_idx) {
                mr.min = r1.dot(mu[j]);
                mr.max = r0.dot(mu[j]);
                nr.min = l1.dot(nu[j]);
                nr.max = l2.dot(nu[j]);
            }
        }
    }

    return md;
}

std::vector<Sofa*> Sofa::a_priori_sofas(
        std::vector<Coord> normals,
        std::size_t mu_fix_idx,
        std::size_t n) 
{
    std::vector<Sofa*> sofas(n);
    SofaMetadata md = SofaMetadata::a_priori_sofa_metadata(
            normals, mu_fix_idx, n);
    for (std::size_t i = 0; i < n; i++) {
        sofas[i] = new Sofa(normals, 
                md.init_params[i].mu_range,
                md.init_params[i].nu_range,
                mu_fix_idx);
    }
    return sofas;
}

std::vector<Coord> Sofa::mu_to_nu(
        std::vector<Coord> mu)
{
    for (auto &v : mu)
        v = Coord(-v.y, v.x);
    return mu;
}

std::vector<BandPair> Sofa::make_band_pairs(
        const std::vector<Coord> &mu,
        const std::vector<Coord> &nu,
        const std::vector<Interval> &mu_range,
        const std::vector<Interval> &nu_range,
        std::size_t mu_fix_idx)
{
    std::size_t n = mu.size();
    assert(n == nu.size());
    assert(n == mu_range.size());
    assert(n == nu_range.size());
    std::vector<BandPair> res;
    for (std::size_t i = 0; i < n; i++) {
        if (i == mu_fix_idx) {
            // mu should be fixed for such index
            assert(mu_range[i].min == mu_range[i].max);
            res.emplace_back(mu[i],
                    mu_range[i].min,
                    mu_range[i].min + 1_mpq/3_mpz, //auxiliary
                    mu_range[i].min + 2_mpq/3_mpz, //auxiliary
                    mu_range[i].min + 1_mpq);
            continue;
        }
        assert(mu_range[i].min < mu_range[i].max);
        res.emplace_back(mu[i], 
                mu_range[i].min,
                mu_range[i].avg(),
                mu_range[i].avg() + 1,
                mu_range[i].max + 1);
    }
    res.emplace_back(0_mpq, 
            0_mpq, 1_mpq/3_mpz, 2_mpq/3_mpz, 1_mpq);
    for (std::size_t i = 0; i < n; i++) {
        assert(nu_range[i].min < nu_range[i].max);
        res.emplace_back(nu[i], 
                nu_range[i].min,
                nu_range[i].avg(),
                nu_range[i].avg() + 1,
                nu_range[i].max + 1);
    }
    return res;
}

Sofa::Sofa(
        std::vector<Coord> normals,
        std::vector<Interval> mu_range,
        std::vector<Interval> nu_range,
        std::size_t mu_fix_idx) :
    n(normals.size()),
    mu_fix_idx(mu_fix_idx),
    mu(normals), 
    nu(mu_to_nu(normals)),
    mu_range(mu_range), 
    nu_range(nu_range),
    ctx(make_band_pairs(mu, nu, mu_range, nu_range, mu_fix_idx)),
    polygons(),
    area()
{
    for (const auto &coord : normals) {
        assert(coord.x > 0);
        assert(coord.y > 0);
        assert(coord.x * coord.x + coord.y * coord.y == 1);
    }
    Coord pivot = ctx.intersection(
            luu(mu_fix_idx), ruu(mu_fix_idx));
    assert(pivot.y > 0);
    polygons = {{short(~luu(mu_fix_idx)), hl(), short(~ruu(mu_fix_idx))}};
    polygons = HalfPlaneRegion(ctx, short(~hu())).intersection(polygons);
    for (std::size_t i = 0; i < n; i++) {
        polygons = UnionOfTwoHalfPlanesRegion(
                ctx, ldd(i), rdd(i)).intersection(polygons);
        polygons = HalfPlaneRegion(ctx, short(~luu(i))).intersection(polygons);
        polygons = HalfPlaneRegion(ctx, short(~ruu(i))).intersection(polygons);
    }

    area = calc_area(polygons);
}



Sofa::Sofa(
        const Sofa &other, 
        std::size_t idx,
        HalveType t) :
    n(other.n),
    mu_fix_idx(other.mu_fix_idx),
    mu(other.mu),
    nu(other.nu),
    mu_range(other.mu_range), // to be updated
    nu_range(other.nu_range), // to be updated
    ctx(other.ctx, (is_mu(t) ? idx : n + 1 + idx), halve_dir(t)),
    polygons(other.polygons), // to be updated
    area() // to be updated
{
    if (idx == mu_fix_idx) {
        assert(t != kMuDown && t != kMuUp);
    }

    // update mu_range and nu_range
    if (t == kMuDown) {
        mu_range[idx].max = mu_range[idx].avg();
    } else if (t == kMuUp) {
        mu_range[idx].min = mu_range[idx].avg();
    } else if (t == kNuDown) {
        nu_range[idx].max = nu_range[idx].avg();
    } else if (t == kNuUp) {
        nu_range[idx].min = nu_range[idx].avg();
    }

    // update polygons
    if (t == kMuDown) {
        polygons = HalfPlaneRegion(other.ctx, 
                short(~rud(idx))).intersection(polygons);
    } else if (t == kMuUp) {
        polygons = UnionOfTwoHalfPlanesRegion(other.ctx,
                ldd(idx), rdu(idx)).intersection(polygons);
    } else if (t == kNuDown) {
        polygons = HalfPlaneRegion(other.ctx,
                short(~lud(idx))).intersection(polygons);
    } else if (t == kNuUp) {
        polygons = UnionOfTwoHalfPlanesRegion(other.ctx,
                ldu(idx), rdd(idx)).intersection(polygons);
    }
    // convert back to this context
    for (auto &poly : polygons) {
        for (auto &id : poly) {
            bool flip = (id < 0);
            if (flip)
                id = ~id;
            // if id == 1, 2 (mod 4), change to 0 or 3
            if (id % 4 == 1)
                id = id - 1;
            else if (id % 4 == 2)
                id = id + 1;
            if (flip)
                id = ~id;
        }
    }

    area = calc_area(polygons);
}

mpq_class Sofa::halve_gain(
        std::size_t idx,
        HalveType t)
{
    if (t == kMuDown) {
        return calc_area(
                HalfPlaneRegion(ctx, rud(idx)).intersection(polygons)
                );
    } else if (t == kMuUp) {
        auto p = HalfPlaneRegion(ctx, short(~ldd(idx))).intersection(polygons);
        p = HalfPlaneRegion(ctx, short(~rdu(idx))).intersection(p);
        return calc_area(p);
    } else if (t == kNuDown) {
        return calc_area(
                HalfPlaneRegion(ctx, lud(idx)).intersection(polygons)
                );
    } else { // (t == kNuUp) 
        auto p = HalfPlaneRegion(ctx, short(~rdd(idx))).intersection(polygons);
        p = HalfPlaneRegion(ctx, short(~ldu(idx))).intersection(p);
        return calc_area(p);
    }
}

// LineId - Coord conversions

std::vector<Coord> Sofa::poly_to_coord(Polygon p)
{
    if (p.size() == 0)
        return {};

    std::size_t p_size = p.size();
    std::size_t prev_idx = p_size - 1;
    std::vector<Coord> coord_poly(p_size);
    for (std::size_t i = 0; i < p_size; i++) {
        coord_poly[i] = 
            ctx.intersection(p[i], p[prev_idx]);
        prev_idx = i;
    }
    return coord_poly;
}

mpq_class Sofa::calc_area(Polygon p)
{
    std::vector<Coord> coord_p = poly_to_coord(p);
    if (coord_p.size() == 0)
        return 0_mpq;

    mpq_class res = 0_mpq;
    std::size_t p_size = p.size();
    std::size_t prev_idx = p_size - 1;
    for (std::size_t i = 0; i < p_size; i++) {
        Coord &c0 = coord_p[prev_idx];
        Coord &c1 = coord_p[i];
        prev_idx = i;
        res += c0.x * c1.y - c0.y * c1.x;
    }
    return res/2_mpz;
}

mpq_class Sofa::calc_area(Polygons p)
{
    mpq_class res = 0_mpq;
    for (auto &pp : p)
        res += calc_area(pp);
    return res;
}

std::vector< std::vector<Coord> > Sofa::coord_polygons()
{
    std::vector< std::vector<Coord> > coord_polys;
    for (auto &poly : polygons) {
        coord_polys.push_back(poly_to_coord(poly));
    }
    return coord_polys;
}


};
};
