#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <cassert>
#include <tuple>
#include <gmp.h>
#include <gmpxx.h>
#include <thread>
#include <future>
#include <mutex>
#include <utility>

#include "sofa.hpp"

// Program initialization constants

// Number of workers
const std::size_t num_threads = 30;
// Each worker does iteration up to this number 
// then redistributes all sofas to workers
const std::size_t num_iter_per_batch = 10000;

using namespace sofa_designer::sofa;

std::vector<Coord> init_normals()
{
    int n;
    gmp_scanf(" Number of angles: %d", &n);
    std::vector<Coord> normals(n);
    for (std::size_t i = 0; i < normals.size(); i++) {
        mpz_t a, b, c;
        mpz_inits(a, b, c, NULL);
        gmp_scanf(" %Zd %Zd %Zd", a, b, c);
        mpz_class aa(a), bb(b), cc(c);
        mpz_clears(a, b, c, NULL);
        assert(aa*aa+bb*bb==cc*cc);
        normals[i] = Coord(mpq_class(aa,cc), mpq_class(bb,cc));
    }
    return normals;
}

std::tuple<Sofa*, Sofa*> branch(Sofa *s, std::size_t mu_fix_idx)
{
    bool is_mu = false;
    std::size_t max_idx = 0;
    mpq_class gain = s->halve_gain(max_idx, kNuDown);
    for (std::size_t i = 0; i < s->n; i++) {
        for (auto t : {kMuDown, kMuUp, kNuDown, kNuUp}) {
            if (i == mu_fix_idx && Sofa::is_mu(t))
                continue;
            if (gain < s->halve_gain(i, t)) {
                is_mu = Sofa::is_mu(t);
                gain = s->halve_gain(i, t);
                max_idx = i;
            }
        }
    }

    assert(gain > 0);

    if (is_mu)
    {
        Sofa *sd = new Sofa(*s, max_idx, kMuDown);
        Sofa *su = new Sofa(*s, max_idx, kMuUp);
        assert(sd->area + s->halve_gain(max_idx, kMuDown) == s->area);
        assert(su->area + s->halve_gain(max_idx, kMuUp) == s->area);
        return std::make_tuple(sd, su);
    }
    else
    {
        Sofa *sd = new Sofa(*s, max_idx, kNuDown);
        Sofa *su = new Sofa(*s, max_idx, kNuUp);
        assert(sd->area + s->halve_gain(max_idx, kNuDown) == s->area);
        assert(su->area + s->halve_gain(max_idx, kNuUp) == s->area);
        return std::make_tuple(sd, su);
    }
}

// loop for sofa thread
// gets the list of pointers to sofas to divide
// returns the sofas undone and number of iterations
std::tuple< std::vector<Sofa*>, std::size_t > sofa_thread(
        std::vector<Sofa*> sofas, 
        mpq_class target, 
        std::size_t mu_fix_idx, 
        std::size_t thread_idx)
{
    static std::mutex mtx;
    unsigned long long iter_cnt = 0;
    while (sofas.size() && iter_cnt < num_iter_per_batch) {
        Sofa *s = *sofas.rbegin();
        sofas.pop_back();
        if (s->area < target)
            delete s;
        else {
            Sofa *s1, *s2;
            std::tie(s1, s2) = branch(s, mu_fix_idx);
            delete s;
            for (Sofa *cs : {s1, s2}) {
                if (cs->area < target) {
                    delete cs;
                } else {
                    sofas.push_back(cs);
                }
            }
        }
        iter_cnt++;
        if (iter_cnt % 1000U == 0) {
            mtx.lock();
            std::cout << "thread " << thread_idx << std::endl;
            std::cout << "iter_cnt: " << iter_cnt;
            std::cout << " depth: " << sofas.size() << std::endl;
            std::cout << (*sofas.rbegin())->area.get_d() << std::endl;
            Sofa &s = **sofas.rbegin();
            for (Interval i : s.mu_range)
                std::cout << "[" << i.max << ", " <<  i.min << "]" << ", ";
            std::cout << "\n";
            for (Interval i : s.nu_range)
                std::cout << "[" << i.max << ", " <<  i.min << "]" << ", ";
            std::cout << "\n";
            std::cout << "\n";
            mtx.unlock();
        }
    }
    return std::make_pair(std::move(sofas), iter_cnt);
}

int main(int argc, const char * argv[])
{
    // Get input

    std::vector<Coord> normals = init_normals();
    std::size_t mu_fix_idx;
    gmp_scanf(" Index to fix mu: %lu", &mu_fix_idx);
    std::size_t num_sofas;
    gmp_scanf(" Number of initial sofas: %lu", &num_sofas);

    mpq_t target_t;
    mpq_init(target_t);
    gmp_scanf(" Target: %Qd", target_t);
    mpq_class target(target_t);
    mpq_clear(target_t);

    gmp_printf("Using the following normal vectors:\n\n");
    for (const Coord &c : normals)
        std::cout << c << ", " << std::endl;
    gmp_printf("\n");
    gmp_printf("Number of initial sofas: %lu\n", num_sofas);
    gmp_printf("Target: %Qd\n", target.get_mpq_t());

    // Initial sofas
    gmp_printf("\nInitializing...\n\n");
    std::vector<Sofa*> sofas = Sofa::a_priori_sofas(normals, mu_fix_idx, num_sofas);

    // Run each batch through threads
    std::size_t batch_num = 1;
    std::size_t total_iter_cnt = 0;
    while (sofas.size()) {
        std::cout << "Batch #" << batch_num;
        std::cout << " Total iteration: " << total_iter_cnt << std::endl;

        // distribute the sofas to task_sofas
        std::vector<Sofa*> task_sofas[num_threads];
        for (std::size_t i = 0; i < sofas.size(); i++)
            task_sofas[i % num_threads].push_back(sofas[i]);
        sofas.clear();

        // send divided task_sofas to workers
        std::future< std::tuple< std::vector<Sofa*>, std::size_t > > 
            done_sofas[num_threads];
        for (std::size_t i = 0; i < num_threads; i++)
            done_sofas[i] = std::async(sofa_thread, 
                    task_sofas[i], 
                    target, 
                    mu_fix_idx, 
                    i);

        // gather sofas back to list
        sofas.clear();
        for (std::size_t i = 0; i < num_threads; i++) {
            std::vector<Sofa*> done_sofa;
            std::size_t iter_cnt;
            std::tie(done_sofa, iter_cnt) = done_sofas[i].get();
            sofas.insert(sofas.end(), done_sofa.begin(), done_sofa.end());
            total_iter_cnt += iter_cnt;
        }
        
        batch_num++;
    }
    gmp_printf("Done.\n");
    std::cout << "Total iteration: " << total_iter_cnt << std::endl;
    return 0;
}
